package org.selfbus.updater;

import ch.qos.logback.classic.Level;
import org.apache.commons.cli.ParseException;
import org.fusesource.jansi.AnsiConsole;
import org.selfbus.updater.bootloader.BootloaderStatistic;
import org.selfbus.updater.devicemgnt.DeviceManagement;
import org.selfbus.updater.devicemgnt.DeviceManagementFactory;
import org.selfbus.updater.logging.LoggingManager;
import org.selfbus.updater.progress.AnsiCursor;
import org.selfbus.updater.upd.UPDCommand;
import org.selfbus.updater.upd.UPDProtocol;
import tuwien.auto.calimero.*;
import org.selfbus.updater.bootloader.BootDescriptor;
import org.selfbus.updater.bootloader.BootloaderIdentity;
import org.selfbus.updater.bootloader.BootloaderUpdater;
import org.selfbus.updater.upd.UDPProtocolVersion;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import static org.fusesource.jansi.Ansi.*;
import static org.selfbus.updater.devicemgnt.DeviceManagement.getExceptionMessage;
import static org.selfbus.updater.logging.Color.*;
import static org.selfbus.updater.logging.LoggingManager.CONSOLE_APPENDER_NAME;
import static org.selfbus.updater.Utils.shortenPath;

import org.selfbus.updater.gui.GuiMain;
import tuwien.auto.calimero.serial.KNXPortClosedException;

import java.net.UnknownHostException;

/**
 * A Tool for updating the firmware of a Selfbus device in a KNX network.
 * <p>
 * {@link Updater} is a {@link Runnable} tool implementation allowing
 * a user to update Selfbus KNX devices.<br>
 * <br>
 * This tool supports KNX network access using a KNXnet/IP, USB, FT1.2 or TPUART
 * connection. It uses the {@link DeviceManagement} to access the Selfbus KNX device.
 * <p>
 * When running this tool from the console, the {@code main} method of this class is invoked,
 * otherwise use this class in the context appropriate to a {@link Runnable}.<br>
 */
public class Updater implements Runnable {
    @SuppressWarnings("unused")
    private Updater() {} // disable default constructor

    private final static String UPDATER_README_LINK = "https://github.com/selfbus/software-arm-lib/blob/main/firmware_updater/updater/README.md";

    private final static Logger logger = LoggerFactory.getLogger(Updater.class);
    private CliOptions cliOptions = null;
    private static DeviceManagement dm = null;

    /**
     * Constructs an instance of the {@link #Updater} class.
     *
     * @param cliOptions the command-line options to be used
     */
    public Updater(CliOptions cliOptions) {
        logger.debug(ToolInfo.getFullInfo());
        logger.debug(Settings.getLibraryHeader(false));
        this.cliOptions = cliOptions;
    }

    public static void initJansi() {
        AnsiConsole.systemInstall();
    }

    public static void finalizeJansi() {
        System.out.print(AnsiCursor.on()); // make sure we enable the cursor
        AnsiConsole.systemUninstall();
    }

    public static void main(final String[] args) {
        initJansi();

        if (args.length == 0) {
            if (!LoggingManager.isRunningInIntelliJ()) {
                logger.info("GUI start => console output set to log level {}", Level.WARN);
                LoggingManager.setThresholdFilterLogLevel(CONSOLE_APPENDER_NAME, Level.WARN);
            }
            GuiMain.startSwingGui();
            finalizeJansi();
            return;
        }

        try {
            logger.info("{}{}{}", ansi().fgBright(OK).bold(), ToolInfo.getToolAndVersion(), ansi().reset());
            // read in user-supplied command line options
            CliOptions options = new CliOptions(args, ToolInfo.getToolJarName() ,
                    "Selfbus KNX-Firmware update tool options", "");
            if (options.getVersionIsSet()) {
                logger.info("{}{}{}", ansi().fgBright(OK).bold(), Credits.getAsciiLogo(), ansi().reset());
                logger.info("{}{}{}", ansi().fgBright(OK).bold(), Credits.getAuthors(), ansi().reset());
                ToolInfo.showVersion();
                finalizeJansi();
                return;
            }

            if (options.getHelpIsSet()) {
                logger.info(options.helpToString());
                finalizeJansi();
                return;
            }

            if (options.getDiscoverIsSet()) {
                logger.info("Discovering KNX network...");
                DiscoverKnxInterfaces.logIPInterfaces(DiscoverKnxInterfaces.getAllnetIPInterfaces());
                DiscoverKnxInterfaces.logUSBInterfaces(DiscoverKnxInterfaces.getUsbInterfaces());
                DiscoverKnxInterfaces.logCOMPorts(DiscoverKnxInterfaces.getCOMPorts());
                finalizeJansi();
                return;
            }

            final Updater updater = new Updater(options);
            final ShutdownHandler shutdownHandler = new ShutdownHandler().register();
            updater.run();
            shutdownHandler.unregister();
        }
        catch (KNXFormatException | ParseException | InterruptedException e) {
            logExceptionUserFriendly(e);
        }
        finally {
            if (dm != null) {
                dm.close();
            }
            finalizeJansi();
            logger.debug("main exit");
        }
    }

    private static final class ShutdownHandler extends Thread {
        private final Thread t = Thread.currentThread();

        ShutdownHandler register() {
            Runtime.getRuntime().addShutdownHook(this);
            logger.trace("ShutdownHandler registered");
            return this;
        }

        void unregister() {
            try {
                Runtime.getRuntime().removeShutdownHook(this);
                logger.trace("ShutdownHandler unregistered");
            }
            catch (IllegalStateException ignored) {
                // Shutdown is currently in progress
            }
        }

        @Override
        public void run() {
            try {
                t.interrupt();
                logger.trace("ShutdownHandler called");
                if (dm != null) {
                    dm.close();
                    dm = null;
                }
            }
            catch (Exception e) {
                logger.error("Error while shutting down", e);
            }
        }
    }

    /**
     * Minimum delay between two UPDCommand.SEND_DATA telegrams in milliseconds
     */
    public static final int DELAY_MIN = 0;
    /**
     * Maximum delay between two UPDCommand.SEND_DATA telegrams in milliseconds
     */
    public static final int DELAY_MAX = 500;
    /**
     *  Physical address the bootloader is using
     */
    public static final IndividualAddress PHYS_ADDRESS_BOOTLOADER = new IndividualAddress(15, 15,192);
    /**
     *  Physical address the Selfbus Updater is using
     */
    public static final IndividualAddress PHYS_ADDRESS_OWN = new IndividualAddress(0, 0,0);

    /*
     * (non-Javadoc)
     *
     * @see java.lang.Runnable#run()
     */
    @Override
    public void run() {
        try {
            final String hexFileName = cliOptions.getFileName();
            BinImage newFirmware = null;

            if (!hexFileName.isEmpty()) {
                // check if the firmware file exists
                if (!Utils.fileExists(hexFileName)) {
                    throw new UpdaterException(String.format("File '%s' does not exist!", cliOptions.getFileName()));
                }
                // Load Firmware hex file
                logger.info("Loading file '{}'", hexFileName);
                newFirmware = BinImage.readFromHex(hexFileName);
                // Check for APP_VERSION string in new firmware
                if (newFirmware.getAppVersion().isEmpty()) {
                    throw new UpdaterException("Missing APP_VERSION string in firmware!");
                }
            }
            else {
                logger.info("{}No firmware file (*.hex) specified! Specify with --{}{}",
                        ansi().fgBright(WARN), CliOptions.OPT_LONG_FILENAME, ansi().reset());
                logger.info("{}Reading only device information{}", ansi().fgBright(INFO), ansi().reset());
            }

            dm = DeviceManagementFactory.getDeviceManagement(cliOptions);

            logger.debug("Telegram priority: {}", cliOptions.getPriority());
            dm.openLink();
            logger.info("KNX connection: {}", dm.getLinkInfo());

            IndividualAddress deviceAddress = cliOptions.getDevicePhysicalAddress();
            IndividualAddress progDeviceAddress = cliOptions.getProgDevicePhysicalAddress();
            IndividualAddress deviceInProgMode = startIntoBootLoader(deviceAddress, progDeviceAddress);

            dm.openDevice(deviceInProgMode);

            byte[] uid;
            if (cliOptions.getUid().isEmpty()) {
                uid = dm.requestUIDFromDevice();
            } else {
                uid = UPDProtocol.uidToByteArray(cliOptions.getUid());
                if (uid == null) {
                    uid = dm.requestUIDFromDevice();
                }
            }

            dm.unlockDeviceWithUID(uid);

            if ((cliOptions.getDumpFlashStartAddress() >= 0) && (cliOptions.getDumpFlashEndAddress() >= 0)) {
                logger.warn("{}Dumping flash content range {} to bootloader's serial port.{}",
                        ansi().fgBright(OK),
                        String.format("0x%04X-0x%04X", cliOptions.getDumpFlashStartAddress(), cliOptions.getDumpFlashEndAddress()),
                        ansi().reset());
                dm.dumpFlashRange(cliOptions.getDumpFlashStartAddress(), cliOptions.getDumpFlashEndAddress());
                return;
            }

            BootloaderIdentity bootLoaderIdentity = dm.requestBootloaderIdentity();

            // Request current main firmware boot descriptor from device
            BootDescriptor bootDescriptor = dm.requestBootDescriptor();
            if (newFirmware != null) {
                logger.info("New firmware:              {}", newFirmware);
            }

            logger.debug("Requesting APP_VERSION");
            String appVersion = dm.requestAppVersionString();
            if (appVersion.isEmpty()) {
                logger.info("Current APP_VERSION: {}invalid{} ", ansi().fgBright(WARN), ansi().reset());
            } else {
                logger.info("Current APP_VERSION: {}{}{}", ansi().fgBright(OK), appVersion, ansi().reset());
            }

            //  From here on we need a valid firmware
            if (newFirmware == null) {
                if (deviceAddress != null) {
                    dm.restartProgrammingDevice();
                }
                // to get here `uid == null` must be true, so it's fine to exit with no-error
                dm.close();
                return;
            }

            // store new firmware bin file in cache directory
            String cacheFileName = FlashDiffMode.createCacheFileName(newFirmware.startAddress(), newFirmware.length(), newFirmware.crc32());
            BinImage imageCache = BinImage.copyFromArray(newFirmware.getBinData(), newFirmware.startAddress());
            imageCache.writeToBinFile(cacheFileName);

            logger.info("File APP_VERSION   : {}{}{}", ansi().fgBright(OK), newFirmware.getAppVersion(), ansi().reset());

            // Check if FW image has correct offset for MCUs bootloader size
            if (newFirmware.startAddress() < bootLoaderIdentity.applicationFirstAddress()) {
                logger.error("{}Error! The specified firmware image would overwrite parts of the bootloader. Check FW offset setting in the linker!{}",
                        ansi().fgBright(WARN), ansi().reset());
                throw new UpdaterException(String.format("Firmware needs to start at or beyond 0x%04X",
                    bootLoaderIdentity.applicationFirstAddress()));
            }
            else if (newFirmware.startAddress() == bootLoaderIdentity.applicationFirstAddress()) {
                logger.debug("Firmware starts directly beyond bootloader.");
            }
            else {
                logger.debug("Info: There are {} bytes of unused flash between bootloader and firmware.",
                        newFirmware.startAddress() - bootLoaderIdentity.applicationFirstAddress());
            }

            if (cliOptions.getEraseFullFlashIsSet()) {
                logger.warn("{}Deleting the entire flash except from the bootloader itself!{}",
                        ansi().fgBright(WARN), ansi().reset());
                dm.eraseFlash();
            }

            boolean diffMode = false;
            if (!(cliOptions.getFlashingFullModeIsSet())) {
                if (bootDescriptor.valid()) {
                    diffMode = FlashDiffMode.setupDifferentialMode(bootDescriptor);
                }
                else {
                    logger.warn("{}  BootDescriptor is not valid -> switching to full mode{}",
                            ansi().fgBright(WARN), ansi().reset());
                }
            }

            if ((bootLoaderIdentity.versionMajor()) <= 1 && (bootLoaderIdentity.versionMinor() < 20)) {
                dm.setProtocolVersion(UDPProtocolVersion.UDP_V0);
            }
            else {
                dm.setProtocolVersion(UDPProtocolVersion.UDP_V1);
            }

            if (!dm.setBlockSize(cliOptions.getBlockSize())) {
                logger.info("{}Connected bootloader doesn't support block size {}. Using {} bytes.{}",
                        ansi().fgBright(INFO), cliOptions.getBlockSize(), dm.getBlockSize(), ansi().reset());
            }

            if (!cliOptions.getNoFlashIsSet()) { // is flashing firmware disabled? for debugging use only!
                // Start to flash the new firmware
                ResponseResult resultTotal;
                if (diffMode && FlashDiffMode.isInitialized()) {
                    logger.warn("{}Differential mode is EXPERIMENTAL -> Use with caution.{}",
                            ansi().fgBright(WARN), ansi().reset());
                    resultTotal = FlashDiffMode.doDifferentialFlash(dm, newFirmware.startAddress(), newFirmware.getBinData());
                }
                else {
                    logger.debug("Starting FlashFullMode");
                    resultTotal = FlashFullMode.doFullFlash(dm, newFirmware, cliOptions.getDelayMs(), !cliOptions.getEraseFullFlashIsSet(), cliOptions.getLogStatisticsIsSet());
                }
                BootloaderStatistic bootloaderStatistic = dm.requestBootLoaderStatistic();
                if (bootloaderStatistic != null) {
                    logger.info("Bootloader: #Disconnect: {} #repeated T_ACK: {}",
                            bootloaderStatistic.getDisconnectCountColored(), bootloaderStatistic.getRepeatedT_ACKcountColored());
                }
                BootloaderStatistic updaterStatistic = new BootloaderStatistic((int)resultTotal.dropCount(),
                        (int)resultTotal.timeoutCount());
                logger.info("Updater   : #Disconnect: {} #repeated T_ACK: {}",
                        updaterStatistic.getDisconnectCountColored(), updaterStatistic.getRepeatedT_ACKcountColored());
            }
            else {
                logger.warn("--{} => {}only boot description block will be written{}", CliOptions.OPT_LONG_NO_FLASH,
                        ansi().fgBright(WARN), ansi().reset());
            }

            BootDescriptor newBootDescriptor = new BootDescriptor(newFirmware.startAddress(),
                    newFirmware.endAddress(), (int) newFirmware.crc32(), newFirmware.getAppVersionAddress());
            logger.info("Updating boot descriptor with {}", newBootDescriptor);
            dm.programBootDescriptor(newBootDescriptor, cliOptions.getDelayMs());
            logger.info("Finished programming device {}{}{} with '{}{}{}'",
                    ansi().fgBright(INFO), deviceInProgMode, ansi().reset(),
                    ansi().fgBright(INFO), shortenPath(cliOptions.getFileName(), 1), ansi().reset());
            dm.restartProgrammingDevice();
            dm.close();

            logger.debug("Wait {} second(s) for potential Bootloader Updater to finish its job",
                    String.format("%.2f", BootloaderUpdater.BOOTLOADER_UPDATER_MAX_RESTART_TIME_MS / 1000.0f));
            Thread.sleep(BootloaderUpdater.BOOTLOADER_UPDATER_MAX_RESTART_TIME_MS);

            logger.info("Device {}{}{} update completed {}successfully{}.",
                    ansi().fgBright(OK), deviceInProgMode, ansi().reset(), ansi().fgBright(OK), ansi().reset());
        }
        catch (final InterruptedException | IllegalStateException e) {
            Thread.currentThread().interrupt();
            logger.info("{}Update canceled.", System.lineSeparator());
            logger.debug("", e); // todo see logback issue https://github.com/qos-ch/logback/issues/876
        }
        catch (final KNXPortClosedException e) {
            logger.error("", e); // todo see logback issue https://github.com/qos-ch/logback/issues/876
            logger.error("Update failed.");
            if (e.getMessage().contains("error sending report over USB") &&
               (!cliOptions.getUsbVendorIdAndProductId().isBlank()) &&
                System.getProperty("os.name").startsWith("Windows")) {
                logger.info("{}Make sure the USB Interface {} uses the WinUSB driver.{}",
                        ansi().fgBright(WARN), cliOptions.getUsbVendorIdAndProductId(), ansi().reset() );
                logger.info("Checkout {}{}{} for more info.",
                        ansi().fgBright(INFO), UPDATER_README_LINK, ansi().reset() );
            }
        }
        catch (final UpdaterException | KNXException e) {
            logExceptionUserFriendly(e);
        }
        catch (final Throwable e) {
            logger.error("", e); // todo see logback issue https://github.com/qos-ch/logback/issues/876
            logger.error("Update did not finish.");
        }
        finally {
            if (dm != null) {
                dm.close();
            }
        }
    }

    public String requestUid() throws KNXException, UpdaterException, UnknownHostException {
        try {
            dm = new DeviceManagement(cliOptions);
            dm.openLink();

            final IndividualAddress deviceAddress = cliOptions.getDevicePhysicalAddress();
            final IndividualAddress progDeviceAddress = cliOptions.getProgDevicePhysicalAddress();

            final IndividualAddress deviceInProgMode = startIntoBootLoader(deviceAddress, progDeviceAddress);
            dm.openDevice(deviceInProgMode);

            byte[] uid = dm.requestUIDFromDevice();

            dm.unlockDeviceWithUID(uid);
            dm.requestBootloaderIdentity();
            dm.requestBootDescriptor();
            String appVersion = dm.requestAppVersionString();
            if (appVersion.isEmpty()) {
                logger.info("APP_VERSION: {}invalid{} ", ansi().fgBright(WARN), ansi().reset());
            } else {
                logger.info("APP_VERSION: {}{}{}", ansi().fgBright(OK), appVersion, ansi().reset());
            }

            if (deviceAddress != null) {
                dm.restartProgrammingDevice();
            }
            dm.close();
            return UPDProtocol.byteArrayToHex(uid);
        }
        catch (final InterruptedException e) {
            logger.info("requestUid canceled.");
            Thread.currentThread().interrupt();
            return "";
        }
        catch (UpdaterException | KNXException e) {
            logger.error("{}An error occurred while retrieving the UID. {}{}{}",
                    ansi().fgBright(WARN), System.lineSeparator(), e, ansi().reset());
            throw e;
        }
    }

    private static void logExceptionUserFriendly(Throwable e) {
        logger.error("{}{}{} ({})", ansi().fgBright(WARN), e.getMessage(), ansi().reset(),
                e.getClass().getSimpleName());
        Throwable cause = e.getCause();
        if (cause != null) {
            logger.error("{}Caused by: {}{} ({})", ansi().fgBright(WARN), cause.getMessage(), ansi().reset(),
                    cause.getClass().getSimpleName());
        }
        logger.debug("", e); // todo see logback issue https://github.com/qos-ch/logback/issues/876
    }

    public IndividualAddress startIntoBootLoader(IndividualAddress device, IndividualAddress progDevice)
            throws KNXException, UpdaterException, InterruptedException {
        IndividualAddress deviceInProgMode;
        if (device == null) {
            // Only option --progDevice is set => check if device is already in programming mode
            logger.debug("Check if progDevice {} is in programming mode", progDevice);
            deviceInProgMode = dm.checkDevicesInProgrammingMode(progDevice);
            return deviceInProgMode;
        }

        // Option --device handling
//        if (dm.isAddressOccupied(device)) {
//            logger.debug("device {} responded to APCI_DEVICEDESCRIPTOR_READ_PDU", device);
            IndividualAddress[] devicesInProgMode = dm.listDevicesInProgrammingMode();
            if  (devicesInProgMode.length == 0) {
                // No devices in progMode
                logger.debug("Starting device {} into bootloader mode", device);
                dm.restartDeviceToBootloader(device); // Try to restart the device into bootloader mode
                deviceInProgMode = dm.checkDevicesInProgrammingMode(device, progDevice);
            }
            else if ((devicesInProgMode.length == 1) &&
                    ((devicesInProgMode[0].equals(device)) || (devicesInProgMode[0].equals(progDevice)))) {
                logger.debug("device/progDevice {} is already in programming mode", devicesInProgMode[0]);
                deviceInProgMode = devicesInProgMode[0];
            }
            else {
                // Multiple devices are in progMode or device/progDevice is not in progMode
                throw new UpdaterException(getExceptionMessage(devicesInProgMode,
                        new IndividualAddress[]{device, progDevice}));
            }
//        }
//        else {
//            logger.info("{}Device {} is not responding or is running legacy bootloader!{}",
//                    ansi().fgBright(INFO), device, ansi().reset());
//            // todo this is old behavior, but in case of the wrong --device we can also land here with new behavior
//            dm.restartDeviceToBootloader(device);
//            deviceInProgMode = dm.checkDevicesInProgrammingMode(progDevice);
//            if (deviceInProgMode == progDevice) {
//                logger.warn("{}Falling back to --progDevice {}!{}", ansi().fgBright(WARN), progDevice, ansi().reset());
//            }
//        }

        return deviceInProgMode;
    }
}
