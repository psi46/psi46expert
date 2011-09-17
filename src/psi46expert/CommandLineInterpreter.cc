#include "CommandLineInterpreter.h"
#include <stdio.h>
#include <string.h>


CommandLineInterpreter::CommandLineInterpreter()
{}


CommandLineInterpreter::~CommandLineInterpreter()
{}


void CommandLineInterpreter::SetString(const char *aString) {par = aString;}

// == Helper functions from Beat ================================================================


int CommandLineInterpreter::GetHex(char ch)
{
	if (isNumber(ch)) return ch - '0';
	if ('A'<=ch && ch<='F') return ch - 'A' + 10;
	if ('a'<=ch && ch<='f') return ch - 'a' + 10;
	return -1;
}


bool CommandLineInterpreter::GetNumber(int &value)
{
	int i, x;
	int base;
	int cnt;

	bool neg = false;
	if (par[0] == '-') { neg = true; par++; }
	else if (par[0] == '+') par++;

	switch (par[0])
	{
	case 'b':
	case 'B': base =  2; cnt = 31; par++; break;
	case '$': base = 16; cnt =  7; par++; break;
	default:  base = 10; cnt =  9;
	}

	value = GetHex(par[0]);
	if (value < 0 || base <= value) return false;
	par++;

	for (i=0; i<cnt; i++)
	{
		x = GetHex(par[0]);
		if (0 <= x && x < base) value = value*base + x;
		else break;
		par++;
	}

	if (neg) value = -value;

	x = GetHex(par[0]);
	if (0 <= x && x < base) return false;

	return true;
}


bool CommandLineInterpreter::GetInt(int &value, int min, int max)
{
	int v;

	while (isWhitespace(par[0])) par++;

	GetNumber(v);

	if (v < min || max < v) return false;

	value = v;
	return true;
}


bool CommandLineInterpreter::GetIntRange(int &valuemin, int &valuemax, int skipmin, int skipmax)
{  // nn:nn

	int vmin, vmax;

	// scan first number
	while (isWhitespace(par[0])) par++;
	if (par[0] != ':')
	{
		if (!GetInt(vmin, -10000, 10000)) return false;
	}
	else vmin = skipmin;

	// scan for ":"
	if (par[0] == ':')
	{ // scan second number
		par++;
		if (isWhitespace(par[0]) || (par[0] == 0) || (par[0] == '\n')) vmax = skipmax;
		else
		{
			if (!GetInt(vmax, -10000, 10000)) return false;
		}

	}
	else vmax = vmin;

	// check range
	if (vmin>vmax) return false;

	if (vmax<skipmin || vmin>skipmax) return false;

	valuemin = (vmin<skipmin)? skipmin : vmin;
	valuemax = (vmax>skipmax)? skipmax : vmax;
	return true;
}


bool CommandLineInterpreter::GetString(char *value)
{
	while (isWhitespace(par[0])) par++;

	int i = 0;
	while ( !isWhitespace(par[0]) && (par[0] != 0) && (par[0] != '\n'))
	{
		value[i] = par[0];
		par++;
		i++;
	}
	value[i] = 0;
	return i > 0;
}
