from pathlib import Path
import unittest


PROJECT = Path(__file__).resolve().parents[1]
PORT_DIR = PROJECT / "third_party" / "chiaki-ng" / "ps3"


class ChiakiPs3BuildManifestTests(unittest.TestCase):
    def test_port_manifest_declares_lan_direct_core_without_gui_or_relay(self):
        manifest = (PORT_DIR / "sources.mk").read_text(encoding="utf-8")
        source_list = manifest.split("# These files are intentionally", 1)[0]
        config = (PORT_DIR / "include" / "chiaki" / "config.h").read_text(encoding="utf-8")

        for source in (
            "lib/src/session.c",
            "lib/src/streamconnection.c",
            "lib/src/videoreceiver.c",
            "lib/src/frameprocessor.c",
            "lib/src/ctrl.c",
            "lib/src/rpcrypt.c",
            "lib/src/ecdh.c",
            "lib/src/fec.c",
        ):
            self.assertIn(source, source_list)

        for forbidden in ("gui/", "cli/", "remote/holepunch.c", "remote/rudp.c"):
            self.assertNotIn(forbidden, source_list)

        self.assertIn("#define CHIAKI_LIB_ENABLE_OPUS 0", config)
        self.assertIn("#define CHIAKI_LIB_ENABLE_FFMPEG_DECODER 0", config)
        self.assertIn("#define CHIAKI_PS3_LOCAL_LAN_ONLY 1", config)

    def test_platform_makefile_builds_the_first_ps3_static_library(self):
        makefile = (PORT_DIR / "Makefile").read_text(encoding="utf-8")

        self.assertIn("libchiaki-ps3-platform.a", makefile)
        for source in ("lib/src/sock.c", "lib/src/log.c", "$(CURDIR)/src/thread.c", "$(CURDIR)/src/time.c"):
            self.assertIn(source, makefile)

    def test_ps3_overrides_monotonic_time_and_nonportable_thread_apis(self):
        makefile = (PORT_DIR / "Makefile").read_text(encoding="utf-8")
        thread_port = (PORT_DIR / "src" / "thread.c").read_text(encoding="utf-8")
        time_port = (PORT_DIR / "src" / "time.c").read_text(encoding="utf-8")

        self.assertIn("$(CURDIR)/src/thread.c", makefile)
        self.assertIn("$(CURDIR)/src/time.c", makefile)
        self.assertIn("sysGetSystemTime", time_port)
        self.assertNotIn("pthread_clockjoin_np", thread_port)

    def test_makefile_has_a_ps3_fec_library_for_video_recovery(self):
        makefile = (PORT_DIR / "Makefile").read_text(encoding="utf-8")

        self.assertIn("libchiaki-ps3-fec.a", makefile)
        self.assertIn("third-party/gf-complete/src/gf.c", makefile)
        self.assertIn("third-party/jerasure/src/jerasure.c", makefile)

    def test_makefile_generates_and_cross_compiles_takion_protobuf(self):
        makefile = (PORT_DIR / "Makefile").read_text(encoding="utf-8")

        self.assertIn("libchiaki-ps3-protobuf.a", makefile)
        self.assertIn("protoc", makefile)
        self.assertIn("nanopb_generator.py", makefile)
        self.assertIn("takion.pb.c", makefile)

    def test_crypto_library_uses_ps3_rng_and_mbedtls_backend(self):
        makefile = (PORT_DIR / "Makefile").read_text(encoding="utf-8")
        config = (PORT_DIR / "include" / "chiaki" / "config.h").read_text(encoding="utf-8")
        random_port = (PORT_DIR / "src" / "random.c").read_text(encoding="utf-8")
        ecdh_port = (PORT_DIR / "src" / "ecdh.c").read_text(encoding="utf-8")

        self.assertIn("libchiaki-ps3-crypto.a", makefile)
        for source in ("lib/src/rpcrypt.c", "lib/src/gkcrypt.c", "$(CURDIR)/src/random.c", "$(CURDIR)/src/ecdh.c"):
            self.assertIn(source, makefile)
        self.assertIn("#define CHIAKI_LIB_ENABLE_MBEDTLS 1", config)
        self.assertIn("sysGetRandomNumber", random_port)
        self.assertIn("sysGetRandomNumber", ecdh_port)
        self.assertNotIn("mbedtls_entropy_init", ecdh_port)
        gkcrypt = (PROJECT / "third_party" / "chiaki-ng" / "lib" / "src" / "gkcrypt.c").read_text(encoding="utf-8")
        self.assertNotIn("#include <chiaki/session.h>", gkcrypt)
        utils = (PROJECT / "third_party" / "chiaki-ng" / "lib" / "src" / "utils.h").read_text(encoding="utf-8")
        self.assertIn("CHIAKI_PS3_LOCAL_LAN_ONLY", utils)

    def test_session_library_manifest_is_lan_first_and_video_only(self):
        makefile = (PORT_DIR / "Makefile").read_text(encoding="utf-8")
        manifest = (PORT_DIR / "sources.mk").read_text(encoding="utf-8")
        self.assertIn("libchiaki-ps3-session.a", makefile)
        for source in ("lib/src/session.c", "lib/src/streamconnection.c", "lib/src/videoreceiver.c", "lib/src/frameprocessor.c"):
            self.assertIn(source, manifest)
        config = (PORT_DIR / "include" / "chiaki" / "config.h").read_text(encoding="utf-8")
        self.assertIn("CHIAKI_PS3_LOCAL_LAN_ONLY", config)


if __name__ == "__main__":
    unittest.main()
