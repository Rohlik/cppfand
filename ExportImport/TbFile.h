#pragma once

#include <cstdio>
#include <string>
#include "TyFile.h"
#include "../cppfand/constants.h"

class TbFile : TyFile
{
public:
	TbFile(bool noCompress);
	~TbFile();

	FILE* Handle = nullptr;
	std::string Dir;
	std::string FName;
	std::string Ext;
    int Size, OrigSize, SpaceOnDisk;

    void TestErr();
    void Reset();
    void Rewrite();
    void ReadBuf2() override;
    void WriteBuf2() override;
    void BackupH();
    void RestoreH();
    void BackupHFD(WORD h);
    void RestoreHFD(WORD h);
    void BackupFD();
    void RestoreFD();
	void Backup(bool isBackup, WORD Ir);
};

