//! @file debug.cpp
//! @brief DebugLog class definition

//--------------------------------------------------------------------------------
// 
// OpenXOPS
// Copyright (c) 2014-2023, OpenXOPS Project / [-_-;](mikan) All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, 
//   this list of conditions and the following disclaimer in the documentation 
//   and/or other materials provided with the distribution.
// * Neither the name of the OpenXOPS Project nor the names of its contributors 
//   may be used to endorse or promote products derived from this software 
//   without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenXOPS Project BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//--------------------------------------------------------------------------------

#include "debug.h"

#ifdef ENABLE_DEBUGLOG

//! @brief Constructor
DebugLog::DebugLog()
{
	//Generate the filename.
	char time[32];
	GetTimeName(time);
	sprintf(fname, GAMENAME"_log-%s.html", time);

	OutputFlag = false;
	error = false;
}

//! @brief Destructor
DebugLog::~DebugLog()
{
	//Add a footer to the file if the output flag is enabled
	if( OutputFlag == true ){
		FILE *fp;

		fp = fopen(fname, "a");
		if( fp == NULL ){ return; }

		fprintf(fp, "</table>\n");
		fprintf(fp, "</body>\n");
		fprintf(fp, "</html>\n");

		fclose(fp);
	}
}

//! @brief Enable output of log files
//! @warning If this function is not executed, the class itself will not work.
void DebugLog::EnableOutputLog()
{
	//Enable output flag
	OutputFlag = true;
}

//! @brief Change the file name of the log file
//! @param name File Name
void DebugLog::SetLogName(const char* name)
{
	strcpy(fname, name);
}

//! @brief Create a new log file
//! @return Success:false Fail:true
//! @warning Call this after EnableOutputLog().
//! @attention If the file already exists, it will be overwritten and saved. Existing files will be lost.
bool DebugLog::MakeLog()
{
	//If the output flag is not enabled, it will not be processed
	if( OutputFlag == false ){ return true; }

	FILE *fp;

	time_t timer;
	struct tm *local;

	//Get Date
	timer = time(NULL);
	local = localtime(&timer);


	fp = fopen(fname, "w");
	if( fp == NULL ){ return true; }

	//Header
	fprintf(fp, "<html>\n");
	fprintf(fp, "<head>\n");
	fprintf(fp, "<title>%s %s Debug Log</title>\n", GAMENAME, GAMEVERSION);
	fprintf(fp, "<style type=text/css>\n");
	fprintf(fp, "<!--//\n");
	fprintf(fp, "body{ font-size : 12px; }\n");
	fprintf(fp, "table { font-size : 12px; }\n");
	fprintf(fp, "//-->\n");
	fprintf(fp, "</style>\n");
	fprintf(fp, "</head>\n");

	fprintf(fp, "<body>\n");

	//Top of page
	fprintf(fp, "<p>\n");
	fprintf(fp, "<b>%s %s Debug Log</b><br>\n", GAMENAME, GAMEVERSION);
	fprintf(fp, "Date and time : %04d-%02d-%02d %02d:%02d:%02d<br>\n", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	fprintf(fp, "</p>\n");

	fprintf(fp, "<table>\n");

	fprintf(fp, "<tr><td><b><font size=-2>System time (ms)</font></b></td><td><b>Title</b></td><td colspan=3><b>Message</b></td></tr>\n");

	fclose(fp);

	//Enable output flag
	//OutputFlag = true;

	return false;
}

//! @brief Add to log file
//! @param tag Constant representing output mode
//! @param title Title.
//! @param id ID
//! @return Success:false Fail:true
//! @attention Overload WriteLog(int tag, const char* title, const char* text).
bool DebugLog::WriteLog(int tag, const char* title, int id)
{
	if( title == NULL ){ return true; }

	//If the output flag is not enabled, it will not be processed
	if( OutputFlag == false ){ return true; }

	char str[16];
	sprintf(str, "IDÅF%d", id);

	//Overload
	return WriteLog(tag, title, str);
}

//! @brief Add to log file
//! @param tag Constant representing output mode
//! @param title Title.
//! @param text Message body.
//! @return Success:false Fail:true
//! @attention If the previous LOG_INIT or LOG_LOAD was not followed by LOG_COMPLETE, record LOG_ERROR information first.
//! @attention In other words, when initialization or loading is disabled, if the next completion is not recorded, it will automatically be recorded as an error.
bool DebugLog::WriteLog(int tag, const char* title, const char* text)
{
	if( title == NULL ){ return true; }
	if( text == NULL ){ return true; }

	//If the output flag is not enabled, it will not be processed
	if( OutputFlag == false ){ return true; }

	FILE *fp;

	char tagcolor[32];
	char tagstr[32];

	//If you have sent an error directly to the completion information or the error flag,
	if( (tag == LOG_COMPLETE)||(tag == LOG_ERROR) ){
		error = false;
	}

	//If an error flag is enabled, it will be recorded as an error.
	if( error == true ){
		error = false;
		WriteLog(LOG_ERROR, "", "");
	}

	//Call for characters and colors from constants
	switch(tag){
		case LOG_CHECK:
			strcpy(tagcolor, "008000");
			strcpy(tagstr, "Information");
			break;
		case LOG_INIT:
			strcpy(tagcolor, "FF0080");
			strcpy(tagstr, "Initialize");
			break;
		case LOG_LOAD:
			strcpy(tagcolor, "0000FF");
			strcpy(tagstr, "Open-Load");
			break;
		case LOG_CLEANUP:
			strcpy(tagcolor, "FF8000");
			strcpy(tagstr, "Close-Release");
			break;
		case LOG_COMPLETE:
			strcpy(tagcolor, "008000");
			strcpy(tagstr, "Success");
			break;
		case LOG_ERROR:
			strcpy(tagcolor, "FF0000");
			strcpy(tagstr, "Failed");
			break;
		default:
			strcpy(tagcolor, "000000");
			strcpy(tagstr, "");
	}

	fp = fopen(fname, "a");
	if( fp == NULL ){ return true; }

	//Current milliseconds
	fprintf(fp, "<tr><td>Å@%uÅ@</td>", GetTimeMS());

	if( (tag != LOG_COMPLETE)&&(tag != LOG_ERROR) ){
		//General Information
		fprintf(fp, "<td width=100><b><font color=#%s>%s</font></b></td>", tagcolor, tagstr);
		fprintf(fp, "<td width=100>%s</td><td colspan=3>%s</td>", title, text);
	}
	else{
		//Complete / Error Information
		fprintf(fp, "<td colspan=2></td><td width=100><b><font color=#%s>%s</font></b></td>", tagcolor, tagstr);
		fprintf(fp, "<td>%s</td><td>%s</td>", title, text);
	}

	fprintf(fp, "</tr>\n");

	fclose(fp);

	//Initialize or load errors flags
	if( (tag == LOG_INIT)||(tag == LOG_LOAD) ){
		error = true;
	}

	return false;
}

#endif	//ENABLE_DEBUGLOG