#include <unistd.h>
#include <time.h>
#include <stdio.h>

#include "interface/Delay.h"
#include "interface/Log.h"

Delay::Delay() {
	uscount=3000ul;
}


// == Delays ================================================

// -- Microseconds delay
void Delay::Udelay(unsigned int us)
{
	unsigned long long int t;
	t = Rdtsc() + us*uscount;
	while (Rdtsc() < t);
}


// -- Milliseconds delay
void Delay::Mdelay(unsigned int ms)
{
	usleep(ms*1000);
}

// == Calibration ===========================================


unsigned int Delay::Usecs()
{
	unsigned long long t;
	t = Rdtsc();
	return (unsigned int)(t/uscount);
}


int Delay::Median(unsigned int n, int x[])
{
	if (n==0) return 0;
	if (n==1) return x[0];

	// sort
	bool swap;
	do
	{
		swap = false;
		for (unsigned int i=0; i<n-1; i++)
			if (x[i]>x[i+1])
			{
				int temp = x[i];
				x[i] = x[i+1];
				x[i+1] = temp;
				swap = true;
			}
	} while (swap);

	unsigned int m = n/2;
	if (n&1) return x[m];
	return (x[m+1]+x[m])/2;
}


int Delay::Cpu_MHz()
{
	time_t c1, c2;

	Mdelay(1);
	c2 = clock();
	do c1 = clock(); while (c1==c2);
	QWORD t1 = Rdtsc();
	do c2 = clock(); while (c2==c1);
	QWORD t2 = Rdtsc();
	int MHz = (int)((t2-t1)*CLOCKS_PER_SEC/(c2-c1)/1000000);
	return MHz;
}


int Delay::Calibrate()
{
	const int iter = 10;
	Mdelay(100);
	int MHz[iter];
	for (int i=0; i<iter; i++) MHz[i] = Cpu_MHz();
	uscount = Median(iter,MHz);

	if (uscount <   100) uscount =   100;
	if (uscount > 20000) uscount = 20000;

	return uscount;
}


int Delay::GetCPU_MHz()
{
	return uscount;
}


QWORD Delay::Rdtsc()
{
	unsigned long long int t;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (t));
	return t;
}


// == Runtime analysis =======================================

// -- Prints a timestamp
void Delay::Timestamp()
{
	time_t t;
	struct tm *dt;
	time(&t);
	dt = localtime(&t);

  psi::LogInfo() << "[Delay] Timestamp: " << asctime( dt) << psi::endl;
}


// -- Prints the microseconds since StartUsecs()
void Delay::PrintUsecs()
{
	unsigned int u = Usecs();
	printf("usecs: %u\n",u - usecsStart);
}


// -- Resets the microseconds counter
void Delay::StartUsecs()
{
	Calibrate();
	usecsStart = Usecs();
}
