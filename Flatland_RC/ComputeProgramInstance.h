#pragma once
#include "ProgramInstance.h"

class ComputeProgramInstance :
    public ProgramInstance
{
public:

    ComputeProgramInstance(Program* computeProgram, int executeCount);
    ComputeProgramInstance(Program* computeProgram, int executeCountX, int executeCountY);
    ComputeProgramInstance(Program* computeProgram, int executeCountX, int executeCountY, int executeCountZ);

    virtual ~ComputeProgramInstance();

    void RunCompute();

    void SetExecuteCount(int executeCountX);
    void SetExecuteCount(int executeCountX, int executeCountY);
    void SetExecuteCount(int executeCountX, int executeCountY, int executeCountZ);

private:

    int WorkGroupSizeX = 1;
    int WorkGroupSizeY = 1;
    int WorkGroupSizeZ = 1;

    int DispatchCountX = 1;
    int DispatchCountY = 1;
    int DispatchCountZ = 1;

};

