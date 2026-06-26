package org.selfbus.updater.devicemgnt;


import org.jetbrains.annotations.NotNull;
import org.selfbus.updater.upd.UPDProtocol;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import static org.fusesource.jansi.Ansi.ansi;
import static org.selfbus.updater.devicemgnt.MurmurHash3.murmurHash3_x86_32;
import static org.selfbus.updater.logging.Color.INFO;
import static org.selfbus.updater.upd.UPDProtocol.UID_LENGTH_MAX;

public class UidInfo    {
    private static final Logger logger = LoggerFactory.getLogger(UidInfo.class);

    final byte[] uid;
    final String knxSerial;

    public UidInfo(String uid)
    {
        if (uid.isEmpty())
        {
            this.uid = null;
            this.knxSerial = "";
            return;
        }

        this.uid = uidToByteArray(uid);
        this.knxSerial = getKNXSerialNumberFromUID(this.uid);
    }

    public UidInfo(byte[] uid)
    {
        this.uid = uid;
        this.knxSerial = getKNXSerialNumberFromUID(this.uid);
    }

    @Override
    @NotNull
    public String toString() {
        return String.format("id: %s knxSerial: %s", byteArrayToHex(uid), knxSerial);
    }

    public byte[] getUidBytes() {
        return uid;
    }

    public String getUIDText() {
        return byteArrayToHex(uid);
    }

    private String getKNXSerialNumberFromUID(byte[] uid) {
        if (uid == null)
        {
            return "UID invalid.";
        }

        if (uid.length < UID_LENGTH_MAX) {
            return String.format("UID too short (%d). Length should be %d for KNX serial number generation.",
                    uid.length, UID_LENGTH_MAX);
        }

        if (uid.length > UID_LENGTH_MAX) {
            logger.info("{}UID too long ({}). Length should be {}.{}", ansi().fgBright(INFO),
                    uid.length, UID_LENGTH_MAX, ansi().reset());
        }
        byte[] uidTruncated = new byte[UID_LENGTH_MAX];
        System.arraycopy(uid, 0, uidTruncated, 0, uidTruncated.length);
        return String.format("013A:%08X", murmurHash3_x86_32(uidTruncated, 0));
    }

    public void logKNXSerialNumber() {
        if (!knxSerial.isEmpty()) {
            logger.info(" Ser#: {}{}{} (KNX serial number for usage in ETS >= v6.1.1)",
                    ansi().fgBright(INFO), knxSerial, ansi().reset());
        }
        else {
            logger.info(" Ser#: -");
        }
    }

    public static byte[] uidToByteArray(String str) {
        String[] tokens = str.split(":");
        if (tokens.length < UPDProtocol.UID_LENGTH_USED) {
            logger.warn("{}ignoring --uid {}, wrong size {}, expected {}{}", ansi().fgBright(INFO),
                    str, tokens.length, UPDProtocol.UID_LENGTH_USED, ansi().reset());
            return null;
        }

        byte[] uid = new byte[tokens.length];
        for (int n = 0; n < tokens.length; n++) {
            uid[n] = (byte) Integer.parseUnsignedInt(tokens[n], 16);
        }
        return uid;
    }

    public static String byteArrayToHex(byte[] bytes) {
        if (bytes == null) {
            return "";
        }

        StringBuilder txt = new StringBuilder();
        for (int i = 0; i < bytes.length; i++) {
            if (i != 0) {
                txt.append(":");
            }
            txt.append(String.format("%02X", bytes[i]));
        }
        return txt.toString();
    }
}