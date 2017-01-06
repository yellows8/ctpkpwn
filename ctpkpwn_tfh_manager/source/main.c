#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <3ds.h>

u32 ctpkpwn_NsDataId = 0x4e574b50;

Result install_haxx()
{
	Result ret=0;

	bossContext ctx;
	u8 status=0;
	u32 tmp=0;
	u8 tmpbuf[4] = {0};

	char *taskID = "tmptask";

	u64 cur_programID = 0;

	char tmpstr[256];

	ret = APT_GetProgramID(&cur_programID);
	if(R_FAILED(ret))return ret;

	//TODO: Load the version instead of hard-coding it.
	memset(tmpstr, 0, sizeof(tmpstr));
	snprintf(tmpstr, sizeof(tmpstr)-1, "http://yls8.mtheall.com/boss/ctpkpwn/tfh/%016llx/v2.1.0.bin", (unsigned long long)cur_programID);
	//HTTP is used here since it's currently unknown how to setup a non-default rootCA cert for BOSS.

	printf("Running BOSS setup...\n");

	bossDeleteTask(taskID, 0);
	bossDeleteNsData(ctpkpwn_NsDataId);

	bossSetupContextDefault(&ctx, 60, tmpstr);

	ret = bossSendContextConfig(&ctx);
	if(R_FAILED(ret))printf("bossSendContextConfig returned 0x%08x.\n", (unsigned int)ret);

	if(R_SUCCEEDED(ret))
	{
		ret = bossRegisterTask(taskID, 0, 0);
		if(R_FAILED(ret))printf("bossRegisterTask returned 0x%08x.\n", (unsigned int)ret);

		if(R_SUCCEEDED(ret))
		{
			ret = bossStartTaskImmediate(taskID);
			if(R_FAILED(ret))printf("bossStartTaskImmediate returned 0x%08x.\n", (unsigned int)ret);

			if(R_SUCCEEDED(ret))
			{
				printf("Waiting for the task to finish running...\n");

				while(1)
				{
					ret = bossGetTaskState(taskID, 0, &status, NULL ,NULL);
					if(R_FAILED(ret))
					{
						printf("bossGetTaskState returned 0x%08x.\n", (unsigned int)ret);
						break;
					}
					if(R_SUCCEEDED(ret))
					{
						printf("...\n");
					}

					if(status!=BOSSTASKSTATUS_STARTED)break;

					svcSleepThread(1000000000LL);//Delay 1s.
				}
			}

			if(R_SUCCEEDED(ret) && status==BOSSTASKSTATUS_ERROR)
			{
				printf("BOSS task failed. This usually indicates a network failure.\n");
				ret = -9;
			}

			if(R_SUCCEEDED(ret))
			{
				printf("Reading BOSS content...\n");

				tmp = 0;
				ret = bossReadNsData(ctpkpwn_NsDataId, 0, tmpbuf, sizeof(tmpbuf), &tmp, NULL);
				if(R_FAILED(ret))printf("bossReadNsData returned 0x%08x, transfer_total=0x%x.\n", (unsigned int)ret, (unsigned int)tmp);

				if(R_SUCCEEDED(ret) && tmp!=sizeof(tmpbuf))ret = -10;

				if(R_SUCCEEDED(ret) && memcmp(tmpbuf, "CTPK", 4))ret = -11;

				if(R_FAILED(ret))printf("BOSS data reading failed: 0x%08x.\n", (unsigned int)ret);
			}

			bossDeleteTask(taskID, 0);

			if(R_SUCCEEDED(ret))printf("Done.\n");
		}
	}

	return ret;
}

Result delete_haxx()
{
	//Result ret=0;
	char *taskID = "tmptask";

	printf("Deleting...\n");

	bossDeleteTask(taskID, 0);
	//printf("bossDeleteTask returned 0x%08x.\n", (unsigned int)ret);

	bossDeleteNsData(ctpkpwn_NsDataId);
	//printf("bossDeleteNsData returned 0x%08x.\n", (unsigned int)ret);

	printf("Done.\n");

	return 0;
}

int main(int argc, char **argv)
{
	int redraw = 1;
	Result ret = 0;

	// Initialize services
	gfxInitDefault();

	consoleInit(GFX_TOP, NULL);

	printf("ctpkpwn_tfh_manager %s by yellows8.\n", VERSION);
	printf("Manage ctpkpwn for TLoZ: Tri Force Heroes.\n");

	ret = bossInit(0, true);
	if(R_FAILED(ret))printf("bossInit() failed: 0x%08x.\n", (unsigned int)ret);

	if(ret==0)
	{
		while (aptMainLoop())
		{
			gspWaitForVBlank();
			hidScanInput();

			if(redraw)
			{
				printf("\nPress A to install, X to delete, or B to exit.\n");
				redraw = 0;
			}

			u32 kDown = hidKeysDown();
			if (kDown & KEY_B)
				break;

			if (kDown & KEY_A)
			{
				consoleClear();
				ret = install_haxx();
				redraw = 1;
			}
			else if (kDown & KEY_X)
			{
				consoleClear();
				ret = delete_haxx();
				redraw = 1;
			}

			if(ret!=0)break;
		}
	}

	bossExit();

	if(ret!=0)printf("An error occured. If this is an actual issue not related to user failure, please report this to here if it persists(or comment on an already existing issue if needed), with a screenshot: https://github.com/yellows8/ctpkpwn/issues\n");

	printf("Press the START button to exit.\n");
	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	// Exit services
	gfxExit();
	return 0;
}

