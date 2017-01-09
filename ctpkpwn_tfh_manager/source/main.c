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

Result install_haxx(u64 cur_programID, FS_Archive extdata_arch, char *payload_path)
{
	Result ret=0;

	bossContext ctx;
	u8 status=0;
	u32 tmp=0;
	u8 tmpbuf[4] = {0};

	FILE *f = NULL;
	int fd=0;
	struct stat filestats;
	Handle filehandle=0;
	FS_Path payload_fspath = fsMakePath(PATH_ASCII, payload_path);
	u8 *payload_buffer = NULL;
	u32 payload_size = 0;

	char *taskID = "tmptask";

	char tmpstr[256];

	printf("Loading the payload from SD /otherapp.bin...\n");

	f = fopen("sdmc:/otherapp.bin", "r");
	if(f==NULL)
	{
		printf("Failed to open 'sdmc:/otherapp.bin': %d.\n", errno);
		return errno;
	}

	fd = fileno(f);
	if(fd==-1)
	{
		fclose(f);
		return errno;
	}

	if(fstat(fd, &filestats)==-1)return errno;

	payload_size = filestats.st_size;
	if(payload_size > 0x100000)payload_size = 0x100000;

	payload_buffer = malloc(payload_size);
	if(payload_buffer==NULL)
	{
		return -7;
	}
	memset(payload_buffer, 0, payload_size);

	tmp = fread(payload_buffer, 1, payload_size, f);

	fclose(f);

	if(tmp!=payload_size)
	{
		free(payload_buffer);
		return -2;
	}

	if(R_SUCCEEDED(ret))
	{
		printf("Writing the payload to extdata...\n");

		FSUSER_DeleteFile(extdata_arch, payload_fspath);

		ret = FSUSER_CreateFile(extdata_arch, payload_fspath, 0, payload_size);
		if(R_FAILED(ret))
		{
			printf("Failed to create the extdata payload file: 0x%08x.\n", (unsigned int)ret);
			free(payload_buffer);
			return ret;
		}

		ret = FSUSER_OpenFile(&filehandle, extdata_arch, payload_fspath, FS_OPEN_WRITE, 0);
		if(R_FAILED(ret))
		{
			printf("Failed to open the extdata payload file: 0x%08x\n", (unsigned int)ret);
			free(payload_buffer);
			return ret;
		}

		ret = FSFILE_Write(filehandle, &tmp, 0, payload_buffer, payload_size, FS_WRITE_FLUSH);
		free(payload_buffer);
		if(R_FAILED(ret) || tmp!=payload_size)
		{
			printf("Failed to write the extdata payload file: res=0x%08x, transfer-size=0x%x.\n", (unsigned int)ret, (unsigned int)tmp);
			if(ret==0 && tmp!=payload_size)ret = -2;
		}

		FSFILE_Close(filehandle);
	}

	printf("Running BOSS setup...\n");

	//TODO: Load the version instead of hard-coding it.
	memset(tmpstr, 0, sizeof(tmpstr));
	snprintf(tmpstr, sizeof(tmpstr)-1, "http://yls8.mtheall.com/boss/ctpkpwn/tfh/%016llx/v2.1.0.bin", (unsigned long long)cur_programID);
	//HTTP is used here since it's currently unknown how to setup a non-default rootCA cert for BOSS.

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
		}
	}

	if(R_SUCCEEDED(ret))printf("Done.\n");

	return ret;
}

Result delete_haxx(FS_Archive extdata_arch, char *payload_path)
{
	//Result ret=0;
	char *taskID = "tmptask";

	printf("Deleting BOSS data...\n");

	bossDeleteTask(taskID, 0);

	bossDeleteNsData(ctpkpwn_NsDataId);
	//printf("bossDeleteNsData returned 0x%08x.\n", (unsigned int)ret);

	printf("Deleting payload from extdata...\n");

	FSUSER_DeleteFile(extdata_arch, fsMakePath(PATH_ASCII, payload_path));

	printf("Done.\n");

	return 0;
}

int main(int argc, char **argv)
{
	int redraw = 1;
	Result ret = 0;
	u64 cur_programID=0;
	FS_Archive extdata_arch;
	FS_Path archpath;
	FS_ExtSaveDataInfo extdatainfo;

	// Initialize services
	gfxInitDefault();

	consoleInit(GFX_TOP, NULL);

	printf("ctpkpwn_tfh_manager %s by yellows8.\n", VERSION);
	printf("Manage ctpkpwn_tfh for TLoZ: Tri Force Heroes.\n");

	ret = APT_GetProgramID(&cur_programID);
	if(R_FAILED(ret))printf("Failed to get the current programID: 0x%08x.\n", (unsigned int)ret);

	if(R_SUCCEEDED(ret))
	{
		memset(&extdatainfo, 0, sizeof(extdatainfo));
		extdatainfo.mediaType = MEDIATYPE_SD;
		extdatainfo.saveId = (((u32)cur_programID) & 0x0fffff00) >> 8;

		memset(&archpath, 0, sizeof(FS_Path));
		archpath.type = PATH_BINARY;
		archpath.size = 0xc;
		archpath.data = &extdatainfo;
	}

	if(R_SUCCEEDED(ret))
	{
		ret = FSUSER_OpenArchive(&extdata_arch, ARCHIVE_EXTDATA, archpath);
		if(R_FAILED(ret))printf("Failed to open the extdata: 0x%08x.\n", (unsigned int)ret);
	}

	if(R_SUCCEEDED(ret))
	{
		ret = bossInit(0, true);
		if(R_FAILED(ret))
		{
			FSUSER_CloseArchive(extdata_arch);
			printf("bossInit() failed: 0x%08x.\n", (unsigned int)ret);
		}
	}

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
				ret = install_haxx(cur_programID, extdata_arch, "/payload.bin");
				redraw = 1;
			}
			else if (kDown & KEY_X)
			{
				consoleClear();
				ret = delete_haxx(extdata_arch, "/payload.bin");
				redraw = 1;
			}

			if(ret!=0)break;
		}

		FSUSER_CloseArchive(extdata_arch);
		bossExit();
	}

	if(ret!=0)printf("An error occured(0x%08x). If this is an actual issue not related to user failure, please report this to here if it persists(or comment on an already existing issue if needed), with a screenshot: https://github.com/yellows8/ctpkpwn/issues\n", (unsigned int)ret);

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

