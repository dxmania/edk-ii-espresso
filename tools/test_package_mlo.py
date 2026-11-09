import struct
import unittest

from package_mlo import (
    CONFIG_HEADER_SIZE,
    ENTRY_ADDRESS,
    FD_SIZE,
    IMAGE_HEADER_SIZE,
    PAYLOAD_OFFSET,
    build_mlo,
)


class BuildMloTests(unittest.TestCase):
    def test_builds_rom_header_and_payload_layout(self):
        configuration_header = bytes(range(256)) * 2
        fd_image = bytes([0xA5]) * PAYLOAD_OFFSET + bytes([0x5A]) * (
            FD_SIZE - PAYLOAD_OFFSET
        )

        mlo = build_mlo(fd_image, configuration_header)

        payload_size, entry_address = struct.unpack_from(
            "<II", mlo, CONFIG_HEADER_SIZE
        )
        self.assertEqual(payload_size, FD_SIZE - PAYLOAD_OFFSET)
        self.assertEqual(entry_address, ENTRY_ADDRESS)
        self.assertEqual(mlo[:CONFIG_HEADER_SIZE], configuration_header)
        self.assertEqual(
            mlo[CONFIG_HEADER_SIZE + IMAGE_HEADER_SIZE :],
            fd_image[PAYLOAD_OFFSET:],
        )
        self.assertEqual(len(mlo), FD_SIZE)

    def test_rejects_wrong_fd_size(self):
        with self.assertRaisesRegex(ValueError, "FD image must be exactly"):
            build_mlo(b"", bytes(CONFIG_HEADER_SIZE))

    def test_rejects_wrong_configuration_header_size(self):
        with self.assertRaisesRegex(ValueError, "configuration header must be exactly"):
            build_mlo(bytes(FD_SIZE), b"")


if __name__ == "__main__":
    unittest.main()
