#include "BasePixel/ControlNetwork.h"


// -- Initializes a ControlNetwork with different configurations
// -- See TestControlNetwork for the configurations of the test setups
ControlNetwork::ControlNetwork(TBInterface * aTBInterface, ConfigParameters * aConfigParameters)
{
    configParameters = aConfigParameters;
    tbInterface = aTBInterface;
    nModules = configParameters->nModules;

    for (int i = 0; i < nModules; i++)
    {
        module[i] = new Module(configParameters, 0, tbInterface);
    }

    Initialize();
}


ControlNetwork::~ControlNetwork()
{
    for (int i = 0; i < nModules; i++)
    {
        delete module[i];
    }
}


void ControlNetwork::Initialize()
{
    for (int i = 0; i < nModules; i++)
    {
        module[i]->Initialize();
    }
}


int ControlNetwork::NModules()
{
    return nModules;
}


Module * ControlNetwork::GetModule(int iModule)
{
    return module[iModule];
}


void ControlNetwork::Execute(SysCommand &command)
{
    module[command.module]->Execute(command);

    return;

}


void ControlNetwork::SetDAC(int iModule, int chipId, int reg, int value)
{
    module[iModule]->SetDAC(chipId, reg, value);
}

void ControlNetwork::SetTBM(int iModule, int chipId, int reg, int value)
{
    module[iModule]->SetTBM(chipId, reg, value);
}


void ControlNetwork::SetTrim(int iModule, int iRoc, int iCol, int iRow, int trimBit)
{
    module[iModule]->SetTrim(iRoc, iCol, iRow, trimBit);
}


// -- Write the DAC configurations of all chips of all modules
void ControlNetwork::WriteDACParameterFile(const char * filename)
{

    cout << "ControlNetwork::WriteDACParameterFile " << filename << endl;

    for (int i = 0; i < nModules; i++)
    {
        module[i]->WriteDACParameterFile(filename);
    }
}

// -- Read the DAC configurations of all chips of all modules
void ControlNetwork::ReadDACParameterFile(char * filename)
{
    for (int i = 0; i < nModules; i++)
    {
        module[i]->ReadDACParameterFile(filename);
    }
}


// -- Write the trim configurations of all chips of all modules
void ControlNetwork::WriteTrimConfiguration(char * filename)
{
    for (int i = 0; i < nModules; i++)
    {
        module[i]->WriteTrimConfiguration(filename);
    }
}


// -- Read the trim configurations for all chips of all modules
void ControlNetwork::ReadTrimConfiguration(char * filename)
{
    for (int i = 0; i < nModules; i++)
    {
        module[i]->ReadTrimConfiguration(filename);
    }
}
