// This class is able to parse range expressions (e.g. 4:15).
// Only needed by the GUI

#ifndef COMMANDLINEINTERPRETER
#define COMMANDLINEINTERPRETER

class CommandLineInterpreter
{

public:
	CommandLineInterpreter();
	~CommandLineInterpreter();

	void SetString(const char *string);
	int GetHex(char ch);
	bool GetNumber(int &value);
	bool GetInt(int &value, int min, int max);
	bool GetIntRange(int &valuemin, int &valuemax, int skipmin, int skipmax);
	bool GetString(char *value);
	static bool isAlpha(char ch) { return 'A'<=ch && ch<='Z' || 'a'<=ch && ch<='z'; }
	static bool isNumber(char ch) { return '0'<=ch && ch<='9'; }
	static bool isWhitespace(char ch) { return ch==' ' || ch=='\t'; }
	static bool isAlphaNum(char ch) { return isAlpha(ch) || isNumber(ch); }

protected:

	const char *par;

};


#endif
