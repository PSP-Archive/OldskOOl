//------ Proto
int exit_callback(void);
int CallbackThread(void *arg);
int SetupCallbacks(void);


//------ function
int exit_callback(void)
{
	sceKernelExitGame();

	return 0;
}

int CallbackThread(void *arg)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback);
	sceKernelRegisterExitCallback(cbid);
	sceKernelPollCallbacks();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}
	
	

	return thid;
}
