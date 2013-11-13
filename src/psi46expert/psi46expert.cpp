#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <getopt.h>
#include "config.h"

#include <TFile.h>
#include <TString.h>
#include <TApplication.h>
#include <TStyle.h>

#include "interface/Delay.h"
#include "psi46expert/TestParameters.h"
#include "psi46expert/TestControlNetwork.h"
#include "psi46expert/MainFrame.h"
#include "psi46expert/Xray.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/SysCommand.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/Keithley.h"
#include "interface/Log.h"

/* These inclusions have to be after any ROOT inclusions */
#include <readline/readline.h>
#include <readline/history.h>

TBInterface * tbInterface;
TestControlNetwork * controlNetwork;
ConfigParameters * configParameters;
SysCommand sysCommand;

Keithley * Power_supply;

const char * testMode = "";
char cmdFile[1000];
bool guiMode(false);
int V = 0;

const char * fullTest = "full";
const char * shortTest = "short";
const char * shortCalTest = "shortCal";
const char * calTest = "cal";
const char * phCalTest = "phCal";
const char * dtlTest = "dtlScan";
const char * xrayTest = "xray";

const char * guiTest = "GUI";
const char * scurveTest = "scurves";
const char * preTest = "preTest";
const char * TrimTest = "trimTest";
const char * ThrMaps = "ThrMaps";

int hubId;
char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000], maskFile[1000];
bool rootFileArg(false), dacArg(false), trimArg(false), tbArg(false), logFileArg(false), cmdFileArg(false), hubIdArg(false), maskArg(false);

void runGUI()
{
    TApplication * application = new TApplication("App", 0, 0, 0, -1);
    MainFrame MainFrame(gClient->GetRoot(), 400, 400, tbInterface, controlNetwork, configParameters);
    application->Run();
}


void execute(SysCommand &command)
{
    do
    {
        /* Separate commands with a newline in batch mode */
        if (cmdFileArg)
            psi::LogInfo() << psi::endl;

        if (command.Keyword("gui"))
        {
            runGUI();
        }
        else if (command.TargetIsTB()) {tbInterface -> Execute(command);}
        else  {controlNetwork->Execute(command);}
    }
    while (command.Next());
    tbInterface->Flush();
}



void runTest()
{
    if (tbInterface->IsPresent() < 1)
    {
        cout << "Error!! Testboard not present. Aborting" << endl;
        return;
    }
    gDelay->Timestamp();
    if (strcmp(testMode, fullTest) == 0)
    {
        psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: start." << psi::endl;

        controlNetwork->FullTestAndCalibration();

        psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: end." << psi::endl;
    }
    if (strcmp(testMode, shortTest) == 0)
    {
        psi::LogInfo() << "[psi46expert] SvShortTest: start." << psi::endl;

        controlNetwork->ShortCalibration();

        psi::LogInfo() << "[psi46expert] SvShortTest: end." << psi::endl;
    }
    if (strcmp(testMode, shortCalTest) == 0)
    {
        psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: start." << psi::endl;

        controlNetwork->ShortTestAndCalibration();

        psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: end." << psi::endl;
    }
    if (strcmp(testMode, xrayTest) == 0)
    {
        TestRange * testRange = new TestRange();
        testRange->CompleteRange();
        Test * test = new Xray(testRange, controlNetwork->GetTestParameters(), tbInterface);
        test->ControlNetworkAction(controlNetwork);
    }
    if (strcmp(testMode, calTest) == 0)
    {
        sysCommand.Read("cal.sys");
        execute(sysCommand);
    }
    if (strcmp(testMode, phCalTest) == 0)
    {
        sysCommand.Read("phCal.sys");
        execute(sysCommand);
    }
    if (strcmp(testMode, dtlTest) == 0)
    {
        sysCommand.Read("dtlTest.sys");
        execute(sysCommand);
    }

    if (strcmp(testMode, guiTest) == 0)
    {
        sysCommand.Read("gui.sys");
        execute(sysCommand);
    }

    if (strcmp(testMode, ThrMaps) == 0)
    {
        sysCommand.Read("ThrMaps.sys");
        execute(sysCommand);
    }
    if (strcmp(testMode, scurveTest) == 0)
    {
        sysCommand.Read("scurve.sys");
        execute(sysCommand);
    }

    gDelay->Timestamp();
}


void runFile()
{
    if (tbInterface->IsPresent() < 1)
    {
        psi::LogInfo() << "[psi46expert] Error: Testboard is not present. Abort.";

        return;
    }

    gDelay->Timestamp();

    psi::LogInfo() << "[psi46expert] Executing file '" << cmdFile
                   << "'." << psi::endl;

    int failed = sysCommand.Read(cmdFile);
    if (failed)
        return;
    execute(sysCommand);

    gDelay->Timestamp();
}

/**
    Checks the given directory. If it doesn't exist and it is run
    interactively it will try to copy default parameters from DATADIR
    into it giving the user a selection.
 */
static int check_parameter_dir(const char * directory, bool interactive)
{
    /* Check whether the directory exists */
    struct stat st;
    if (stat(directory, &st) == 0)
        return 1;

    /* It does not exist. If the program is not running interactively, abort. */
    if (!interactive || errno != ENOENT) {
        perror(Form("[psi46expert] Problem accessing test directory '%s'", directory));
        return 0;
    }

    psi::LogInfo() << "[psi46expert] The directory '" << directory << "' does not exist." << psi::endl;

    /* Are there default parameters installed in DATADIR? */
    /* Check whether DATADIR exists */
    if (stat(DATADIR, &st) != 0)
        return 0;

    /* Get the contents of DATADIR */
    struct dirent ** dirlist;
    int ndirs = scandir(DATADIR, &dirlist, NULL, alphasort);
    if (ndirs <= 0)
        return 0;
    int nvalid = 0;
    /* Iterate over the files in DATADIR */
    for (int i = 0; i < ndirs; i++) {
        /* Use only files (directories) that start with "defaultParameters" */
        if (strncmp(dirlist[i]->d_name, "defaultParameters", strlen("defaultParameters")) == 0) {
            if (nvalid == 0)
                psi::LogInfo() << "[psi46expert] The following default parameter sets are available from " << DATADIR << ":" << psi::endl << psi::endl;
            psi::LogInfo() << Form("%4i: ", nvalid + 1) << dirlist[i]->d_name << psi::endl;
            nvalid++;
        }
    }

    /* Are there any valid directories in DATADIR? */
    if (nvalid == 0)
        return 0;
    psi::LogInfo() << psi::endl;

    /* Ask the user which directory to use */
    char * answer;
    answer = readline(Form("Which default parameter set would you like to use? [1-%i]: ", nvalid));
    int number = -1;
    /* Analyse answer */
    if (sscanf(answer, "%i", &number) != 1 || number < 1 || number > nvalid) {
        psi::LogError() << "[psi46expert] Invalid answer!" << psi::endl;
        return 0;
    }
    int selection = 0;
    char * default_parameters = NULL;
    /* Find the directory to which the answer corresponds */
    for (int i = 0; i < ndirs; i++) {
        if (strncmp(dirlist[i]->d_name, "defaultParameters", strlen("defaultParameters")) == 0) {
            selection++;
            if (selection == number) {
                default_parameters = strdup(dirlist[i]->d_name);
                break;
            }
        }
    }

    /* Copy the directory */
    int ret = system(Form("cp -r \"%s/%s\" \"%s\"", DATADIR, default_parameters, directory));

    /* Check copy status */
    if (ret == 0) {
        psi::LogInfo() << "[psi46expert] Copied default parameters " << default_parameters << " into test directory " << directory << "." << psi::endl;

        /* Free the list of directories */
        for (int i = 0; i < ndirs; i++) {
            free(dirlist[i]);
        }
        return 1;
    } else {
        psi::LogError() << "[psi46expert] Error copying files!" << psi::endl;
        return 0;
    }
}

/* Long options for use with getopt. Last entry in the array has to be all zeros. */
static struct option long_options[] = {
    /* Long option, argument type, flag variable, flag value / option id */
    {"dir",         required_argument,  NULL,   256},
    {"root-file",   required_argument,  NULL,   'r'},
    {"test-root",   required_argument,  NULL,   'c'},
    {"dac-file",    required_argument,  NULL,   'd'},
    {"batch-file",  required_argument,  NULL,   'f'},
    {"log",         required_argument,  NULL,   257},
    {"trim",        required_argument,  NULL,   258},
    {"trimVcal",    required_argument,  NULL,   259},
    {"mask",        required_argument,  NULL,   260},
    {"tb",          required_argument,  NULL,   261},
    {"test-mode",   required_argument,  NULL,   't'},
    {"voltage",     required_argument,  NULL,   'V'},
    {"gui",         no_argument,        NULL,   'g'},
    {"help",        no_argument,        NULL,   'h'},
    {0, 0, 0, 0}
};

/* Option descriptions that correspond to the options in long_options. */
static const char * option_descriptions [] = {
    "Test parameter directory (default: testModule)",
    "Output ROOT file name",
    "Output ROOT file name (test-<arg>.root)",
    "DAC parameter file name",
    "Batch processing file name",
    "Log file name",
    "Trim parameter file name",
    "Trim parameter selection (e.g. 60)",
    "Mask file name",
    "Testboard identifier",
    "Test mode",
    "Sensor bias voltage",
    "Start the GUI automatically",
    "Displays this help message",
    NULL
};

/**
    Prints the command line arguments in a nicely formatted way using
    the arrays long_options and option_descriptions.
 */
void print_usage()
{
    int opt = 0;
    int desc = 0;
    int len = 0;
    /* Determine longest argument */
    while (long_options[opt].name) {
        /* Only count options that don't have single character flags */
        if (long_options[opt].val > 255 && strlen(long_options[opt].name) > len)
            len = strlen(long_options[opt].name);
        opt++;
    }

    /* Print arguments and descriptions */
    printf("Usage:\n");
    opt = 0;
    while (long_options[opt].name) {
        int arglen;
        /* Long options have values greater than 255 */
        if (long_options[opt].val > 255) {
            printf("  -%s%*s ", long_options[opt].name, len - strlen(long_options[opt].name), "");
        } else {
            printf("  -%c%*s ", long_options[opt].val, len - 1, "");
            arglen = 1;
        }

        /* Print the argument, if there is one */
        if (long_options[opt].has_arg == required_argument)
            printf("<arg>    ");
        else if (long_options[opt].has_arg == optional_argument)
            printf("[arg]    ");
        else
            printf("         ");

        /* Print the description of the option */
        if (option_descriptions[opt]) {
            printf("%s", option_descriptions[desc]);
            desc++;
        }
        printf("\n");
        opt++;
    }
}

/**
    Parses the command line arguments and sets the parameters in
    the configParameters structure.
 */
int parse_command_line_arguments(int argc, char * argv[])
{
    /* Default directory */
    strcpy(directory, "testModule");

    /* Go through all arguments */
    int opt, index;
    while ((opt = getopt_long_only(argc, argv, "c:d:r:f:t:gV:h", long_options, &index)) != -1) {
        switch(opt) {
            case 256: /* -dir */
                strcpy(directory, optarg);
                break;
            case 'c':
                rootFileArg = true;
                strcpy(rootFile, Form("test-%s.root", optarg));
                break;
            case 'd':
                dacArg = true;
                sprintf(dacFile, "%s", optarg);
                break;
            case 'r':
                rootFileArg = true;
                sprintf(rootFile, "%s", optarg);
                break;
            case 'f':
                cmdFileArg = true;
                sprintf(cmdFile, "%s", optarg);
                break;
            case 257: /* -log */
                logFileArg = true;
                sprintf(logFile, "%s", optarg);
                break;
            case 258: /* -trim */
                trimArg = true;
                sprintf(trimFile, "%s", optarg);
                break;
            case 259: /* -trimVcal */
                trimArg = true;
                dacArg = true;
                int vcal;
                if (sscanf(optarg, "%i", &vcal) != 1) {
                    print_usage();
                    return 0;
                }
                sprintf(trimFile, "%s%i", "trimParameters", vcal);
                sprintf(dacFile, "%s%i", "dacParameters", vcal);
                break;
            case 260: /* -mask */
                maskArg = true;
                strcpy(maskFile, optarg);
                break;
            case 261: /* -tb */
                tbArg = true;
                sprintf(tbName, "%s", optarg);
                break;
            case 't':
                testMode = strdup(optarg);
                if (strcmp(testMode, dtlTest) == 0) {
                    hubIdArg = true;
                    hubId = -1;
                }
                break;
            case 'g':
                guiMode = true;
                break;
            case 'V':
                if (sscanf(optarg, "%i", &V) != 1) {
                    print_usage();
                    return 0;
                }
                break;
            case 'h': /* -help */
                print_usage();
                return 0;
            default:
                print_usage();
                return 0;
        }
    }

    /* Check whether there are more arguments that were not recognized as options */
    if (optind != argc) {
        print_usage();
        return 0;
    }

    /* Check whether gui mode and batch mode are specified at the same time */
    if (guiMode && cmdFileArg) {
        psi::LogError() << "[psi46expert] Batch mode cannot be started together with the GUI." << psi::endl;
        return 0;
    }

    /* Success */
    return 1;
}

int main(int argc, char * argv[])
{
    /* Parse the command line arguments */
    psi::LogInfo() << "[psi46expert] Reading command line arguments ..." << psi::endl;
    if (!parse_command_line_arguments(argc, argv))
        return 1;

    /* Check whether the test directory exists */
    psi::LogInfo() << "[psi46expert] Checking test directory ..." << psi::endl;
    bool interactive = !cmdFileArg;
    if (!check_parameter_dir(directory, interactive))
        return 1;

    /* Create a new config parameters instance */
    configParameters = ConfigParameters::Singleton();

    /* Set the directory in the config parameter structure */
    strcpy(configParameters->directory, directory);

    if (strcmp(testMode, fullTest) == 0)
    {
        logFileArg = true;
        sprintf(logFile, "FullTest.log");
        rootFileArg = true;
        sprintf(rootFile, "FullTest.root");
    }
    if (strcmp(testMode, shortTest) == 0 || strcmp(testMode, shortCalTest) == 0)
    {
        logFileArg = true;
        sprintf(logFile, "ShortTest.log");
        rootFileArg = true;
        sprintf(rootFile, "ShortTest.root");
    }
    else if (strcmp(testMode, calTest) == 0)
    {
        logFileArg = true;
        sprintf(logFile, "Calibration.log");
        rootFileArg = true;
        sprintf(rootFile, "Calibration.root");
    }

    /* Set the log and debug files */
    if (logFileArg)
        configParameters->SetLogFileName(logFile);
    else
        configParameters->SetLogFileName("log.txt");
    configParameters->SetDebugFileName("debug.log");

    psi::LogInfo().setOutput(configParameters->GetLogFileName());
    psi::LogDebug().setOutput(configParameters->GetDebugFileName());

    psi::LogInfo() << "[psi46expert] --------- psi46expert ---------" << psi::endl;
    psi::LogInfo() << "[psi46expert] " << TDatime().AsString() << psi::endl;

    /* Read the config parameters from the configParameters.dat file in the test directory */
    if (!configParameters->ReadConfigParameterFile(Form("%s/configParameters.dat", configParameters->directory)))
        return 1;

    /* Override config parameters from the configParameters.dat file with the command line arguments */
    if (rootFileArg)
        configParameters->SetRootFileName(rootFile);
    if (dacArg)
        configParameters->SetDacParameterFileName(dacFile);
    if (tbArg)
        strcpy(configParameters->testboardName, tbName);
    if (trimArg)
        configParameters->SetTrimParameterFileName(trimFile);
    if (maskArg)
        configParameters->SetMaskFileName(maskFile);
    if (hubIdArg)
        configParameters->hubId = hubId;

    // == Initialization =====================================================================

    TFile * histoFile = new TFile(configParameters->GetRootFileName(), "RECREATE");
    gStyle->SetPalette(1, 0);

    psi::LogInfo() << "[psi46expert] Setting up the testboard interface ..." << psi::endl;
    tbInterface = new TBInterface(configParameters);
    if (!tbInterface->IsPresent()) return -1;
    controlNetwork = new TestControlNetwork(tbInterface, configParameters);

    Power_supply = new Keithley();
    if (V > 0) {
        Power_supply->Open();
        Power_supply->Init();
        int volt = 25, step = 25;
        while (volt < V - 25) {
            Power_supply->SetVoltage(volt, 1);
            volt = volt + step;
            if (volt > 400) {step = 10;}
            if (volt > 600) {step = 5;}
        }
        Power_supply->SetVoltage(V, 4);
    }

    using_history();
    const char * home_directory = getenv("HOME");
    string history_file("");
    if (home_directory)
        history_file += home_directory;
    history_file += "/.psi46expert_history";
    read_history(history_file.c_str());

    if (guiMode) runGUI();
    else if (strcmp(testMode, "") != 0) runTest();
    else if (strcmp(cmdFile, "") != 0) runFile();
    else
    {
        // == CommandLine ================================================================

        char * p;
        char * last = NULL;
        bool finished = false;
        do {
            p = readline("psi46expert> ");
            if (!last || strcmp(p, last) != 0)
                add_history(p);
            free(last);

            psi::LogDebug() << "psi46expert> " << p << psi::endl;

            if (sysCommand.Parse(p)) execute(sysCommand);
            finished = (strcmp(p, "exit") == 0) || (strcmp(p, "q") == 0);
            last = p;
        }
        while (!finished);
        free(last);
    }

    // == Exit ========================================================================

    if (!strcmp(testMode, phCalTest) == 0)
    {
        tbInterface->HVoff();
        tbInterface->Poff();
        tbInterface->Close();
    }

    if (V > 0)
    {
        Power_supply->ShutDown(); }

    delete controlNetwork;
    delete tbInterface;

    histoFile->Write();
    histoFile->Close();
    delete histoFile;
    delete Power_supply;

    write_history(history_file.c_str());

    return 0;
}
