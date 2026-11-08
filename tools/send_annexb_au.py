#!/usr/bin/env python3
"""Send one H.264 Annex-B access unit to a PS3 P3RP/1 prototype receiver.

This is a data-plane test tool. It does not connect to a PS5, Chiaki, PSN, or
Remote Play service. Give it a single complete access unit prepared by a local
encoder/test fixture.
"""

from __future__ import annotations

import argparse
import socket
import struct
from pathlib import Path

MAGIC = b"P3RP"
VERSION = 1
FLAG_IDR = 0x01
VIDEO_PORT = 9295
HEADER_SIZE = 16
MAX_DATAGRAM = 1200
MAX_PAYLOAD = MAX_DATAGRAM - HEADER_SIZE


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("ps3_ipv4")
    parser.add_argument("access_unit", type=Path)
    parser.add_argument("--port", type=int, default=VIDEO_PORT)
    parser.add_argument("--frame-id", type=int, default=1)
    parser.add_argument("--idr", action="store_true")
    args = parser.parse_args()

    au = args.access_unit.read_bytes()
    if not au:
        raise SystemExit("access unit is empty")
    if len(au) > 4 * 1024 * 1024:
        raise SystemExit("access unit exceeds receiver maximum (4 MiB)")

    fragment_count = (len(au) + MAX_PAYLOAD - 1) // MAX_PAYLOAD
    if fragment_count > 0xFFFF:
        raise SystemExit("too many UDP fragments")

    flags = FLAG_IDR if args.idr else 0
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        for index in range(fragment_count):
            offset = index * MAX_PAYLOAD
            payload = au[offset : offset + MAX_PAYLOAD]
            header = MAGIC + struct.pack(
                ">BBIHHH", VERSION, flags, args.frame_id, index, fragment_count, len(payload)
            )
            assert len(header) == HEADER_SIZE
            sock.sendto(header + payload, (args.ps3_ipv4, args.port))

    print(f"sent frame={args.frame_id} bytes={len(au)} fragments={fragment_count} to {args.ps3_ipv4}:{args.port}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
