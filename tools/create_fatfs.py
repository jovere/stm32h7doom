#!/usr/bin/env python3
"""
FAT Filesystem Image Generator using mkfs.vfat
Creates a minimal FAT16 filesystem image with specified files.

Usage:
    python3 create_fatfs.py <output_bin> <input_file> [input_file2 ...]

Example:
    python3 create_fatfs.py qspi_fs.bin doom.wad
    python3 create_fatfs.py qspi_fs.bin doom.wad sprite.bin

Requirements:
    mkfs.vfat (usually pre-installed on Linux)
    python3
"""

import os
import sys
import subprocess
import tempfile
from pathlib import Path
from typing import List, Tuple


def calculate_filesystem_size(file_sizes: List[int]) -> int:
    """
    Calculate filesystem size based on input file sizes.

    Returns size in KB with ~10% overhead for FAT structures.
    """
    total_size = sum(file_sizes)
    overhead = int(total_size * 0.10)  # 10% overhead for FAT, boot sector, etc.

    # Minimum size: 33MB for FAT16 (mkfs.vfat strict requirement for FAT16)
    # FAT16 needs at least 32MB to work reliably
    min_size = 33 * 1024 * 1024
    total = total_size + overhead

    # Convert to KB (mkfs.vfat uses 1KB blocks)
    return (max(total, min_size) + 1023) // 1024


def create_filesystem(output_bin: str, input_files: List[Tuple[str, bytes]]) -> bool:
    """
    Create a FAT filesystem image using mkfs.vfat and PyFatFS.
    Automatically chooses FAT12 for small files, FAT16 for larger ones.

    Args:
        output_bin: Output filesystem image path
        input_files: List of (filename, file_data) tuples

    Returns:
        True if successful, False otherwise
    """
    if not input_files:
        print("Error: No input files provided")
        return False

    # Calculate filesystem size
    file_sizes = [len(data) for _, data in input_files]
    total_input_size = sum(file_sizes)

    # Determine FAT type based on file size
    # FAT12 max: ~32MB (but typically used for <20MB)
    # FAT16 max: ~2GB
    fat12_threshold = 20 * 1024 * 1024  # 20MB threshold for FAT12

    if total_input_size < fat12_threshold:
        # Use FAT12 for small files
        fat_type = '12'
        # FAT12 minimum is much smaller
        min_size = 1 * 1024 * 1024  # 1MB minimum for FAT12
        fs_size_bytes = max(total_input_size * 110 // 100, min_size)  # 10% overhead
        fs_size_kb = (fs_size_bytes + 1023) // 1024
    else:
        # Use FAT16 for larger files
        fat_type = '16'
        fs_size_kb = calculate_filesystem_size(file_sizes)

    print(f"Creating FAT{fat_type} filesystem image: {output_bin}")
    print(f"Total input size: {total_input_size} bytes ({total_input_size / (1024*1024):.2f} MB)")
    print(f"Filesystem size: {fs_size_kb * 1024} bytes ({fs_size_kb / 1024:.2f} MB)")

    try:
        # Check if mkfs.vfat is available
        result = subprocess.run(['mkfs.vfat', '--help'], capture_output=True, text=True)
        if result.returncode != 0:
            print("Error: mkfs.vfat not found. Install it with:")
            print("  sudo apt install dosfstools  # Debian/Ubuntu")
            print("  sudo yum install dosfstools  # RHEL/CentOS")
            print("  brew install dosfstools      # macOS")
            return False

        # Create empty filesystem image
        print(f"\n  Creating filesystem with mkfs.vfat...")

        # Remove old file if it exists
        if os.path.exists(output_bin):
            os.remove(output_bin)
            print(f"  Removed existing file: {output_bin}")

        subprocess.run(
            ['mkfs.vfat', '-F', fat_type, '-n', 'DOOM', '-C', output_bin, str(fs_size_kb)],
            check=True,
            capture_output=True,
            text=True
        )

        # Add files to filesystem using PyFatFS
        try:
            from pyfatfs.PyFatFS import PyFatFS
        except ImportError:
            print("Error: pyfatfs module not found")
            print("Install it with: pip install pyfatfs")
            return False

        with PyFatFS(output_bin) as fs:
            # Add files to filesystem
            for filename, file_data in input_files:
                # PyFatFS handles 8.3 conversion automatically
                # Just use uppercase filename
                name_part = Path(filename).stem.upper()[:8]
                ext_part = Path(filename).suffix[1:].upper()[:3] if Path(filename).suffix else ""

                # Reconstruct with period for PyFatFS
                if ext_part:
                    short_name = f"{name_part}.{ext_part}"
                else:
                    short_name = name_part

                fs_path = f"/{short_name}"

                print(f"  Adding: {filename} → {short_name} ({len(file_data)} bytes)")

                # Write file to filesystem
                fs.writebytes(fs_path, file_data)

        print(f"\n✓ Filesystem created successfully")
        print(f"  Files: {len(input_files)}")
        print(f"  Output: {output_bin}")

        return True

    except subprocess.CalledProcessError as e:
        print(f"Error running mkfs.vfat: {e.stderr}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"Error creating filesystem: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return False


def main():
    if len(sys.argv) < 3:
        print("Usage: python3 create_fatfs.py <output_bin> <input_file> [input_file2 ...]")
        print("Example: python3 create_fatfs.py qspi_fs.bin doom.wad")
        print("")
        print("Requirements:")
        print("  mkfs.vfat (usually pre-installed)")
        print("  python3")
        print("  pip install pyfatfs")
        sys.exit(1)

    output_bin = sys.argv[1]
    input_files_paths = sys.argv[2:]

    # Read input files
    input_files = []
    for input_file in input_files_paths:
        if not os.path.exists(input_file):
            print(f"Error: File not found: {input_file}", file=sys.stderr)
            sys.exit(1)

        with open(input_file, 'rb') as f:
            data = f.read()

        input_files.append((os.path.basename(input_file), data))
        print(f"Loaded: {input_file} ({len(data)} bytes)")

    # Create output directory if needed
    os.makedirs(os.path.dirname(output_bin) or '.', exist_ok=True)

    # Create filesystem
    if create_filesystem(output_bin, input_files):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()
