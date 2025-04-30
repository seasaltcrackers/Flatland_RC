#include "ComputeProgramInstance.h"
#include "Program.h"

ComputeProgramInstance::ComputeProgramInstance(Program* computeProgram, int executeCount) :
	ProgramInstance(computeProgram)
{
	int workGroupSize[3];
	glGetProgramiv(computeProgram->GetProgramID(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);

	WorkGroupSizeX = workGroupSize[0];
	DispatchCountX = (int)std::ceilf(executeCount / (float)workGroupSize[0]);
}

ComputeProgramInstance::ComputeProgramInstance(Program* computeProgram, int executeCountX, int executeCountY) :
	ProgramInstance(computeProgram)
{
	int workGroupSize[3];
	glGetProgramiv(computeProgram->GetProgramID(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);

	WorkGroupSizeX = workGroupSize[0];
	WorkGroupSizeY = workGroupSize[1];

	DispatchCountX = (int)std::ceilf(executeCountX / (float)workGroupSize[0]);
	DispatchCountY = (int)std::ceilf(executeCountY / (float)workGroupSize[1]);
}

ComputeProgramInstance::ComputeProgramInstance(Program* computeProgram, int executeCountX, int executeCountY, int executeCountZ) :
	ProgramInstance(computeProgram)
{
	int workGroupSize[3];
	glGetProgramiv(computeProgram->GetProgramID(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);

	WorkGroupSizeX = workGroupSize[0];
	WorkGroupSizeY = workGroupSize[1];
	WorkGroupSizeZ = workGroupSize[2];

	DispatchCountX = (int)std::ceilf(executeCountX / (float)workGroupSize[0]);
	DispatchCountY = (int)std::ceilf(executeCountY / (float)workGroupSize[1]);
	DispatchCountZ = (int)std::ceilf(executeCountZ / (float)workGroupSize[2]);
}

ComputeProgramInstance::~ComputeProgramInstance()
{
}

void ComputeProgramInstance::RunCompute()
{
	if (DispatchCountX == 0 || DispatchCountY == 0 || DispatchCountZ == 0)
		return;

	Bind();

	glDispatchCompute(DispatchCountX, DispatchCountY, DispatchCountZ);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	Unbind();
}

void ComputeProgramInstance::SetExecuteCount(int executeCountX)
{
	DispatchCountX = (int)std::ceilf(executeCountX / (float)WorkGroupSizeX);
}

void ComputeProgramInstance::SetExecuteCount(int executeCountX, int executeCountY)
{
	DispatchCountX = (int)std::ceilf(executeCountX / (float)WorkGroupSizeX);
	DispatchCountY = (int)std::ceilf(executeCountY / (float)WorkGroupSizeY);
}

void ComputeProgramInstance::SetExecuteCount(int executeCountX, int executeCountY, int executeCountZ)
{
	DispatchCountX = (int)std::ceilf(executeCountX / (float)WorkGroupSizeX);
	DispatchCountY = (int)std::ceilf(executeCountY / (float)WorkGroupSizeY);
	DispatchCountZ = (int)std::ceilf(executeCountZ / (float)WorkGroupSizeZ);
}
