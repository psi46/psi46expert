#include "pipe.h"
#include <cstdlib>

PipeEnd end;

Pipe::Pipe()
{
	source = NULL;
}

PipeObject * Pipe::Read()
{
	return source->Write();
}

PipeObject * Pipe::Write()
{
	if (source != NULL)
		return Read();
	else
		return NULL;
}

Pipe & Pipe::operator>>(Pipe & right)
{
	right.source = this;
	return right;
}

void operator>>(Pipe & left, PipeEnd & right)
{
	right.source = &left;
	right.process();
}

void PipeEnd::process()
{
	while (Read());
}
