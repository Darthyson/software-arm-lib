package org.selfbus.updater;

import org.selfbus.updater.upd.UPDProtocol;
import org.junit.jupiter.api.Test;
import java.util.Arrays;
//import org.jetbrains.annotations.NotNull;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.selfbus.updater.MurmurHash3.murmurHash3_x86_32;
import static org.selfbus.updater.upd.UPDProtocol.UID_LENGTH_MAX;

/**
 * Unit tests for {@link MurmurHash3#murmurHash3_x86_32}.
 * Test data was taken from test_hashing_testcases.h (C++ header).
 */
public class MurmurHash3Test {

    private record HashTestCase(int id, String uid, int expectedMurmurHash3) {
        @Override
        // @NotNull
        public String toString() {
            return String.format("id=%d uid=%s MurmurHash3=%08X", id, uid, expectedMurmurHash3);
        }
    }

    private static HashTestCase[] hashTestCases() {
        return new HashTestCase[]{
            new HashTestCase(0, "4D:D0:03:05:E7:A4:6B:AF:F2:76:72:5E:05:1C:00:F5", 0xAAF1EC0A),
            new HashTestCase(1, "20:50:02:0B:67:98:3D:AF:43:CE:71:5C:03:1C:00:F5", 0x070CE9C9),
            new HashTestCase(2, "37:20:02:0B:67:98:3D:AF:7D:CD:71:5C:04:1C:00:F5", 0x74AAE925),
            new HashTestCase(3, "1F:70:00:14:02:1D:0E:AF:AD:BD:E2:59:05:1C:00:F5", 0xA94A736A),
            new HashTestCase(4, "4D:80:03:05:E7:A4:6B:AF:B5:76:72:5E:04:1C:00:F5", 0x26E2B2E0),
            new HashTestCase(5, "13:50:04:12:49:84:5E:AF:75:9F:F5:5D:06:1C:00:F5", 0x10320323),
            new HashTestCase(6, "25:D0:02:0B:67:98:3D:AF:12:CF:71:5C:07:1C:00:F5", 0x413926F3),
            new HashTestCase(7, "06:30:02:0B:67:98:3D:AF:EC:CC:71:5C:03:1C:00:F5", 0xBBF93067),
            new HashTestCase(8, "1E:A0:02:0B:67:98:3D:AF:00:CF:71:5C:02:1C:00:F5", 0xC101F962),
            new HashTestCase(9, "29:20:01:04:28:9C:AC:AF:86:3C:5A:62:04:1C:00:F5", 0x253939C3),
            new HashTestCase(10, "1F:A0:02:0B:67:98:3D:AF:00:CF:71:5C:05:1C:00:F5", 0xB2E896F0),
            new HashTestCase(11, "25:D0:01:10:C8:10:65:AF:6A:E2:FA:5D:07:1C:00:F5", 0xDEBDBFD7),
            new HashTestCase(12, "46:20:01:04:28:9C:AC:AF:94:3D:5A:62:02:1C:00:F5", 0x5C32327F),
            new HashTestCase(13, "3D:90:02:13:49:84:5E:AF:67:AB:F5:5D:06:1C:00:F5", 0x782B1ABE),
            new HashTestCase(14, "4E:E0:03:05:E7:A4:6B:AF:FF:76:72:5E:03:1C:00:F5", 0xB9108718),
            new HashTestCase(15, "15:20:02:0B:67:98:3D:AF:17:CD:71:5C:06:1C:00:F5", 0x0C55525A),
            new HashTestCase(16, "4D:00:04:05:E7:A4:6B:AF:0D:77:72:5E:04:1C:00:F5", 0xF647D9DA),
            new HashTestCase(17, "1C:00:04:0D:60:84:5E:AF:7B:D5:F8:5D:02:1C:00:F5", 0xFCB469EC),
            new HashTestCase(18, "4D:B0:03:05:E7:A4:6B:AF:B5:76:72:5E:07:1C:00:F5", 0x4EE1EF67),
            new HashTestCase(19, "26:30:01:12:E0:10:65:AF:6E:02:FF:5D:02:1C:00:F5", 0x349F5D66),
            new HashTestCase(20, "24:30:01:12:E0:10:65:AF:68:02:FF:5D:02:1C:00:F5", 0x5B5F05CA),
            new HashTestCase(21, "1E:A0:00:08:27:A0:CD:AE:A1:38:3A:57:87:19:00:F5", 0xA0E1BBA4),
            new HashTestCase(22, "19:D0:02:0B:67:98:3D:AF:EE:CE:71:5C:07:1C:00:F5", 0xC32F4555),
            new HashTestCase(23, "1B:D0:02:0B:67:98:3D:AF:F4:CE:71:5C:07:1C:00:F5", 0x690478E7),
            new HashTestCase(24, "18:50:04:12:49:84:5E:AF:3F:9F:F5:5D:01:1C:00:F5", 0x8C71CD40),
            new HashTestCase(25, "3A:90:02:13:49:84:5E:AF:6D:AB:F5:5D:01:1C:00:F5", 0xE8BBE00E),
            new HashTestCase(26, "0F:30:02:0B:67:98:3D:AF:05:CD:71:5C:06:1C:00:F5", 0x4EA5DA4D),
            new HashTestCase(27, "24:F0:03:0D:60:84:5E:AF:37:D6:F8:5D:02:1C:00:F5", 0x9A0DA432),
            new HashTestCase(28, "40:90:02:13:49:84:5E:AF:5B:AB:F5:5D:00:1C:00:F5", 0x8F75B13E),
            new HashTestCase(29, "4A:20:01:12:E0:10:65:AF:C3:05:FF:5D:03:1C:00:F5", 0x119AEB11),
            new HashTestCase(30, "11:A0:02:0B:67:98:3D:AF:D6:CE:71:5C:05:1C:00:F5", 0x939B1393),
            new HashTestCase(31, "1C:70:00:14:02:1D:0E:AF:B3:BD:E2:59:03:1C:00:F5", 0x8E4D1121),
            new HashTestCase(32, "15:90:02:02:28:9C:AC:AF:52:19:5A:62:05:1C:00:F5", 0x8B784ADC),
            new HashTestCase(33, "12:10:04:0D:60:84:5E:AF:B2:D5:F8:5D:01:1C:00:F5", 0x4BB3B661),
            new HashTestCase(34, "43:90:02:13:49:84:5E:AF:50:AD:F5:5D:04:1C:00:F5", 0x51EC6178),
            new HashTestCase(35, "17:A0:02:0B:67:98:3D:AF:E8:CE:71:5C:05:1C:00:F5", 0x57700F9D),
            new HashTestCase(36, "1C:D0:01:10:C8:10:65:AF:82:E2:FA:5D:01:1C:00:F5", 0xEFE0CA9F),
            new HashTestCase(37, "40:20:01:0E:C8:10:65:AF:DE:BB:FA:5D", 0x15D0FE47),
            new HashTestCase(38, "40:E0:01:16:48:84:5E:AF:3B:0B:FD:5D", 0x2D30F923),
            new HashTestCase(39, "3F:D0:01:16:48:84:5E:AF:41:0B:FD:5D", 0x06DBE614),
            new HashTestCase(40, "3A:20:01:0E:C8:10:65:AF:73:BC:FA:5D", 0xCA9ED307),
            new HashTestCase(41, "3C:D0:03:0E:C8:10:65:AF:31:C3:FA:5D", 0xF5B5B7B3),
            new HashTestCase(42, "45:B0:01:10:C8:10:65:AF:E4:DD:FA:5D", 0x5C18F099),
            new HashTestCase(43, "30:E0:01:16:48:84:5E:AF:8A:0B:FD:5D", 0xBCC7710A),
            new HashTestCase(44, "2C:30:01:0E:C8:10:65:AF:F8:BA:FA:5D", 0xBF0881BB),
            new HashTestCase(45, "54:B0:01:10:C8:10:65:AF:C1:E0:FA:5D", 0x736789AE),
            new HashTestCase(46, "2C:E0:01:16:48:84:5E:AF:C6:0B:FD:5D", 0xA836AE51),
            new HashTestCase(47, "39:30:01:0E:C8:10:65:AF:47:BB:FA:5D", 0xFF2F8EBD),
            new HashTestCase(48, "32:30:01:0E:C8:10:65:AF:35:BB:FA:5D", 0x33EFADAA),
            new HashTestCase(49, "20:20:01:0E:C8:10:65:AF:B0:BA:FA:5D", 0xE749F195),
            new HashTestCase(50, "43:40:01:0E:C8:10:65:AF:E4:BB:FA:5D", 0x654B75AC),
            new HashTestCase(51, "1F:20:01:0E:C8:10:65:AF:AA:BA:FA:5D", 0xC78CE82B),
            new HashTestCase(52, "1B:E0:01:16:48:84:5E:AF:EE:12:FD:5D", 0x0AF8F440),
            new HashTestCase(53, "46:40:01:0E:C8:10:65:AF:24:BC:FA:5D", 0xC9A58771),
            new HashTestCase(54, "4B:40:01:0E:C8:10:65:AF:73:BE:FA:5D", 0x9AA3C489),
            new HashTestCase(55, "49:40:01:0E:C8:10:65:AF:2A:BC:FA:5D", 0x809A73AE),
            new HashTestCase(56, "2D:30:01:0E:C8:10:65:AF:F8:BA:FA:5D", 0x775F4504),
            new HashTestCase(57, "3F:40:01:0E:C8:10:65:AF:D8:BB:FA:5D", 0x2F42E0E0),
            new HashTestCase(58, "44:40:01:0E:C8:10:65:AF:EA:BB:FA:5D", 0x9D0F3A27),
            new HashTestCase(59, "47:40:01:0E:C8:10:65:AF:24:BC:FA:5D", 0x1F561501),
            new HashTestCase(60, "2E:30:01:0E:C8:10:65:AF:FE:BA:FA:5D", 0x3E9F2042),
            new HashTestCase(61, "48:90:03:13:49:84:5E:AF:B1:AD:F5:5D", 0x9D7E5440),
            new HashTestCase(62, "21:70:04:01:28:9C:AC:AF:93:13:5A:62", 0xC3915259),
            new HashTestCase(63, "33:30:01:0E:C8:10:65:AF:35:BB:FA:5D", 0x5D47ABB7),
            new HashTestCase(64, "42:40:01:0E:C8:10:65:AF:E4:BB:FA:5D", 0x234CCB35),
            new HashTestCase(65, "32:40:01:0E:C8:10:65:AF:35:BB:FA:5D", 0x11E9141C),
            new HashTestCase(66, "3E:40:01:0E:C8:10:65:AF:D8:BB:FA:5D", 0x07D7DBD1),
            new HashTestCase(67, "39:40:01:0E:C8:10:65:AF:47:BB:FA:5D", 0x12E16D1E),
            new HashTestCase(68, "3D:40:01:0E:C8:10:65:AF:53:BB:FA:5D", 0x78BD9A7C),
            new HashTestCase(69, "3A:40:01:0E:C8:10:65:AF:4D:BB:FA:5D", 0x651004B6),
            new HashTestCase(70, "40:40:01:0E:C8:10:65:AF:DE:BB:FA:5D", 0xAF0C5E80),
            new HashTestCase(71, "37:B0:01:10:C8:10:65:AF:03:E2:FA:5D", 0x0303D403),
            new HashTestCase(72, "4F:B0:01:10:C8:10:65:AF:DA:E0:FA:5D", 0x2AF21AB6),
            new HashTestCase(73, "41:B0:01:0D:60:84:5E:AF:D2:CC:F8:5D", 0x3C66E6A9),
            new HashTestCase(74, "24:00:04:0D:60:84:5E:AF:37:D6:F8:5D", 0x0C4DA3EC),
            new HashTestCase(75, "42:B0:01:0D:60:84:5E:AF:D8:CC:F8:5D", 0x8B2D5881),
            new HashTestCase(76, "35:C0:01:0D:60:84:5E:AF:1B:D1:F8:5D", 0x51F58399),
            new HashTestCase(77, "4C:C0:01:0D:60:84:5E:AF:F2:CF:F8:5D", 0x76906A75),
            new HashTestCase(78, "4C:B0:01:0D:60:84:5E:AF:0F:CD:F8:5D", 0xA82EADDE),
            new HashTestCase(79, "16:30:01:12:E0:10:65:AF:C5:01:FF:5D", 0x16025BB0),
            new HashTestCase(80, "4A:B0:01:0D:60:84:5E:AF:09:CD:F8:5D", 0xD0F53177),
            new HashTestCase(81, "48:B0:01:0D:60:84:5E:AF:03:CD:F8:5D", 0xAE7D29CE),
            new HashTestCase(82, "39:C0:01:0D:60:84:5E:AF:0F:D1:F8:5D", 0xCC692AC4),
            new HashTestCase(83, "20:00:04:0D:60:84:5E:AF:13:D6:F8:5D", 0x275C2BFF),
            new HashTestCase(84, "41:40:01:0E:C8:10:65:AF:DE:BB:FA:5D", 0xBB9D635B),
            new HashTestCase(85, "35:30:01:0E:C8:10:65:AF:3B:BB:FA:5D", 0x4B6D0924),
            new HashTestCase(86, "3B:40:01:0E:C8:10:65:AF:4D:BB:FA:5D", 0xE1EBBA5E),
            new HashTestCase(87, "28:60:01:02:28:9C:AC:AF:F0:1A:5A:62", 0x27DE921A),
            new HashTestCase(88, "0A:00:03:0B:69:98:35:AF:C8:9A:6A:5B", 0x1A9747D9),
            new HashTestCase(89, "23:E0:00:0B:69:98:35:AF:33:9F:6A:5B", 0xAE26BAD5),
            new HashTestCase(90, "15:E0:00:0B:69:98:35:AF:87:9C:6A:5B", 0xAC11B9EC),
            new HashTestCase(91, "1F:00:04:0F:C8:10:65:AF:F8:D5:FA:5D", 0x2638EF1F),
            new HashTestCase(92, "26:E0:00:0B:69:98:35:AF:27:9F:6A:5B", 0x880A9CFE),
            new HashTestCase(93, "49:B0:01:10:C8:10:65:AF:F0:DD:FA:5D", 0x6FC75139),
        };
    }

    @Test
    public void testMurmurHash3_x86_32() {
        for (HashTestCase tc : hashTestCases()) {
            byte[] uid = UPDProtocol.uidToByteArray(tc.uid());
            assertNotNull(uid, () -> String.format("uid is null for test case %s", tc));
            if (uid.length == UID_LENGTH_MAX - 4) {
                // UID is too short so append the last 4 bytes FF:1C:00:F5
                uid = Arrays.copyOf(uid, UID_LENGTH_MAX);
                uid[UID_LENGTH_MAX - 4] = (byte) 0xFF;
                uid[UID_LENGTH_MAX - 3] = (byte) 0x1C;
                uid[UID_LENGTH_MAX - 2] = (byte) 0x00;
                uid[UID_LENGTH_MAX - 1] = (byte) 0xF5;
            }

            assertEquals(tc.expectedMurmurHash3(), murmurHash3_x86_32(uid, 0),
                    () -> String.format("MurmurHash3 mismatch for test case %s", tc));
        }
    }
}
