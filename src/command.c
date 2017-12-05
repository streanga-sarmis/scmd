//   scmd a simple and small unix shell
//
//	Copyright (C) 2017 Streanga Sarmis-Stefan.
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program, if not, see <http://www.gnu.org/licenses/>.

#include "command/command.h"

void initPipe(){
	pipe(mainPipe);
}


void initCommander(){
	kids = 0;
}

void initForkKid(){
	pid = fork();
	kids++;
}

void waitInput(){

	char* buf = readline("$> ");
	
	parseCommand(buf);

	free(buf);
}


void parseCommand(const char* cmd){

	int checker = 0;
	add_history(cmd);
	
	if(strlen(cmd) != 0){

		char* redirArgs[2];
		
		char* tredir = cmd;
		char* tspaces = cmd;

		int redirects = 0;
		int spaces = 0;

		tredir = strtok(tredir, ">");

		while(tredir != NULL){
			redirArgs[redirects++] = tredir;
			tredir = strtok(NULL, ">");
		}

		char* args[128];

		tspaces = strtok(tspaces, " ");

		while(tspaces != NULL){
			args[spaces++] = tspaces;
			tspaces = strtok(NULL, " ");
		}
		
		
		if (strcmp(cmd, "exit") == 0) commandExit();
		else if(spaces == 1 && redirects == 1) execLinux(cmd);
		else if(redirects > 1){
			printf("this guy wants a redirect to %s\n", redirArgs[1]);
		} else if(spaces > 1 && redirects == 1){
			execParams(args, spaces);
		}

	} else {
		printf("Could not read parse an empty command\n");
	}

}

void execParams(const char* args[128], int spaces){
	
	char** cargs = args; // @FixMe

	if(strcmp(cargs[0], "nl")) commandNl(cargs);
	else {
		int totalSize = 0;

		for(int i = 0; i < spaces + 1; i++){
			totalSize += strlen(cargs[i]);
		}


		char* cmd = (char*)malloc(totalSize * sizeof(char));
		memset(cmd, 0, (totalSize - 1) * sizeof(char));

		for(int i = 0; i < spaces + 1; i++){
			strcat(cmd, cargs[i]);
			printf("%s\n", cmd);
		}

		execLinux(cmd);
	}

}

void execLinux(const char* cmd){

	kids++;
	initForkKid();

	if(pid > 0){
		char* tmp = cmd;
		char* args[12];

		int num = 0;
		tmp = strtok(tmp, " ");

		while(tmp != NULL){
			args[num] = tmp;
			num++;

			tmp = strtok(NULL, " ");
		}
		num++;


		write(mainPipe[1], &num, sizeof(num));
		int i;

		for(i = 0; i < num - 1; i++){
			int size = strlen(args[i]);

			write(mainPipe[1], &size, sizeof(size));
			write(mainPipe[1], args[i], size);
		}

		waitForKids();

		// free(args); @FixMe seg fault


	} else if(pid == 0){


		int num;
		read(mainPipe[0], &num, sizeof(num));

		char** args = (char**)malloc((num) * sizeof(char*));

		int i;
		for(i = 0; i < num - 1; i++){
			if(i == 0){
				int size;
				read(mainPipe[0], &size, sizeof(size));

				args[i] = (char*)malloc(size + 5 + 1);
				memset(args[i], 0, size + 5 + 1);					
	
				char* tmpc = (char*)malloc(size + 1);
				memset(tmpc, 0, size + 1);					
	
				read(mainPipe[0], tmpc, size);

				char* str1 = (char*)malloc(size + 5 + 1);
				memset(str1, 0, size + 5 + 1);					

				str1[0] = '/';
				str1[1] = 'b';
				str1[2] = 'i';
				str1[3] = 'n';
				str1[4] = '/';
	
				strcat(str1, tmpc);
	
				args[i] = str1;
			} else {
				int size;
				read(mainPipe[0], &size, sizeof(size));

				args[i] = (char*)malloc(size + 1);
				memset(args[i], 0, size + 1);					

				read(mainPipe[0], args[i], size);
			}
		}

		args[num - 1] = NULL;

		execvp(args[0], args);

		printf("Could not execute !\n");

		exit(1);
	}

}

void waitForKids(){
	if (getKids() > 0){
		int i = 0;
		for(; i < getKids(); i++) wait();
	}
}

uint32_t getKids(){
	return kids;
}


