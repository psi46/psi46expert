#ifndef __PIPE_H__
#define __PIPE_H__

/* Base class for objects transported in pipes. */
class PipeObject {};

class Pipe {
	public:
		Pipe * source;
	
	public:
		virtual PipeObject * Read();
		virtual PipeObject * Write();
	
	public:
		Pipe();
		Pipe & operator>>(Pipe & right);
};

class PipeEnd : public Pipe {
	private:
		void process();
	friend void operator>>(Pipe & left, PipeEnd & right);
};

extern PipeEnd end;

void operator>>(Pipe & left, PipeEnd & right);

#endif
