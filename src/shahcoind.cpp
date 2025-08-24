#include <init.h>
#include <node/context.h>
#include <interfaces/init.h>
#include <util/system.h>
#include <chainparams.h>
#include <chainparamsbase.h>
#include <noui.h>
#include <common/args.h>
#include <util/check.h>
#include <util/translation.h>
#include <util/strencodings.h>
#include <util/tokenpipe.h>
#include <shutdown.h>
#include <clientversion.h>
#include <tinyformat.h>

static bool ParseArgs(ArgsManager& args, int argc, char* argv[])
{
    SetupServerArgs(args);
    SetupChainParamsBaseOptions(args);
    
    // Convert boolean flags to the expected format before parsing
    std::vector<std::string> converted_args;
    converted_args.push_back(argv[0]); // Keep the program name
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-regtest") {
            converted_args.push_back("-regtest=1");
        } else if (arg == "-testnet") {
            converted_args.push_back("-testnet=1");
        } else if (arg == "-signet") {
            converted_args.push_back("-signet=1");
        } else {
            converted_args.push_back(arg);
        }
    }
    
    // Convert back to char* array for parsing
    std::vector<const char*> char_args;
    for (const auto& arg : converted_args) {
        char_args.push_back(arg.c_str());
    }
    
    // Parse arguments with converted format
    std::string error;
    if (!args.ParseParameters(char_args.size(), char_args.data(), error)) {
        std::cout << "Failed to parse parameters: " << error << std::endl;
        return false;
    }
    
    // Set the config path to avoid the assertion failure
    std::string config_error;
    if (!args.ReadConfigFiles(config_error, true)) {
        std::cout << "Failed to read config files: " << config_error << std::endl;
        // Don't return false here as config files are optional
    }
    
    return true;
}

static bool ProcessInitCommands(ArgsManager& args)
{
    if (HelpRequested(args) || args.IsArgSet("-version")) {
        std::string strUsage = PACKAGE_NAME " version " + FormatFullVersion() + "\n";
        if (args.IsArgSet("-version")) {
            strUsage += FormatParagraph(LicenseInfo());
        } else {
            strUsage += "\nUsage:  shahcoind [options]                     Start " PACKAGE_NAME "\n\n";
            strUsage += args.GetHelpMessage();
        }
        tfm::format(std::cout, "%s", strUsage);
        return true;
    }
    return false;
}

static bool AppInit(node::NodeContext& node)
{
    ArgsManager& args = *Assert(node.args);

    // -server defaults to true for shahcoind but not for the GUI so do this here
    args.SoftSetBoolArg("-server", true);
    // Set this early so that parameter interactions go to console
    InitLogging(args);
    InitParameterInteraction(args);
    if (!AppInitBasicSetup(args, node.exit_status)) {
        return false;
    }
    if (!AppInitParameterInteraction(args)) {
        return false;
    }

    node.kernel = std::make_unique<kernel::Context>();
    if (!AppInitSanityChecks(*node.kernel)) {
        return false;
    }

    // Lock data directory after daemonization
    if (!AppInitLockDataDirectory()) {
        return false;
    }
    return AppInitInterfaces(node) && AppInitMain(node);
}

int main(int argc, char* argv[])
{
    std::cout << "Starting Shahcoin daemon..." << std::endl;
    std::cout << "Number of arguments: " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "Arg " << i << ": " << argv[i] << std::endl;
    }
    
    SetupEnvironment();
    std::cout << "Environment setup complete" << std::endl;

    node::NodeContext node;
    int exit_status;
    std::cout << "Creating NodeContext..." << std::endl;
    
    std::unique_ptr<interfaces::Init> init = interfaces::MakeNodeInit(node, argc, argv, exit_status);
    if (!init) {
        std::cout << "Failed to create init interface, exit status: " << exit_status << std::endl;
        return exit_status;
    }
    std::cout << "Init interface created successfully" << std::endl;

    // Connect shahcoind signal handlers
    noui_connect();
    std::cout << "Signal handlers connected" << std::endl;

    // Interpret command line arguments
    ArgsManager& args = *Assert(node.args);
    std::cout << "Parsing arguments..." << std::endl;
    if (!ParseArgs(args, argc, argv)) {
        std::cout << "Failed to parse arguments" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Arguments parsed successfully" << std::endl;
    
    // Initialize chain parameters before processing commands
    std::cout << "Initializing chain parameters..." << std::endl;
    std::cout << "Raw chain type from args: " << static_cast<int>(args.GetChainType()) << std::endl;
    
    // Check if -regtest argument was passed
    bool is_regtest = args.IsArgSet("-regtest");
    std::cout << "Is -regtest set: " << (is_regtest ? "yes" : "no") << std::endl;
    
    // Also check for -chain=regtest
    bool is_chain_regtest = args.IsArgSet("-chain") && args.GetArg("-chain", "") == "regtest";
    std::cout << "Is -chain=regtest set: " << (is_chain_regtest ? "yes" : "no") << std::endl;
    
    std::string chain_type;
    if (is_regtest || is_chain_regtest) {
        chain_type = "regtest";
    } else {
        switch (args.GetChainType()) {
            case ChainType::MAIN:
                chain_type = "main";
                break;
            case ChainType::TESTNET:
                chain_type = "test";
                break;
            case ChainType::REGTEST:
                chain_type = "regtest";
                break;
            case ChainType::SIGNET:
                chain_type = "signet";
                break;
            default:
                chain_type = "main";
                break;
        }
    }
    std::cout << "Chain type: " << chain_type << std::endl;
    SelectBaseParams(chain_type);
    SelectParams(chain_type);
    std::cout << "Chain parameters initialized" << std::endl;
    
    // Process early info return commands such as -help or -version
    if (ProcessInitCommands(args)) {
        std::cout << "Processed init commands, exiting" << std::endl;
        return EXIT_SUCCESS;
    }

    // Start application
    std::cout << "Starting AppInit..." << std::endl;
    if (AppInit(node)) {
        std::cout << "AppInit successful, waiting for shutdown..." << std::endl;
        WaitForShutdown();
    } else {
        std::cout << "AppInit failed" << std::endl;
        node.exit_status = EXIT_FAILURE;
    }
    Interrupt(node);
    Shutdown(node);

    std::cout << "Daemon exiting with status: " << node.exit_status << std::endl;
    return node.exit_status;
}
