#!/usr/bin/env python3
"""Wrap an EDK II FD with an explicitly supplied OMAP ROM configuration header."""

import argparse
import struct
from pathlib import Path


FD_SIZE = 0x100000
CONFIG_HEADER_SIZE = 0x200
IMAGE_HEADER_SIZE = 8
PAYLOAD_OFFSET = CONFIG_HEADER_SIZE + IMAGE_HEADER_SIZE
ENTRY_ADDRESS = 0x80008208


def build_mlo(fd_image: bytes, configuration_header: bytes) -> bytes:
    if len(fd_image) != FD_SIZE:
        raise ValueError(f"FD image must be exactly {FD_SIZE:#x} bytes")
    if len(configuration_header) != CONFIG_HEADER_SIZE:
        raise ValueError(
            f"OMAP configuration header must be exactly {CONFIG_HEADER_SIZE:#x} bytes"
        )

    payload = fd_image[PAYLOAD_OFFSET:]
    image_header = struct.pack("<II", len(payload), ENTRY_ADDRESS)
    return configuration_header + image_header + payload


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("fd_image", type=Path, help="1 MiB EDK II FD output")
    parser.add_argument(
        "configuration_header",
        type=Path,
        help="verified 512-byte OMAP ROM configuration header for this board",
    )
    parser.add_argument("output", type=Path, help="destination MLO image")
    parser.add_argument(
        "--force", action="store_true", help="replace an existing output file"
    )
    args = parser.parse_args()

    try:
        if args.output.resolve() in {
            args.fd_image.resolve(),
            args.configuration_header.resolve(),
        }:
            parser.error("output must not overwrite either input file")

        mlo = build_mlo(
            args.fd_image.read_bytes(), args.configuration_header.read_bytes()
        )
        if args.force:
            args.output.write_bytes(mlo)
        else:
            with args.output.open("xb") as output_file:
                output_file.write(mlo)
    except (OSError, ValueError) as error:
        parser.error(str(error))

    print(f"Wrote {len(mlo):#x} bytes to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
