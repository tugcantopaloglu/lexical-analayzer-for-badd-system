#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
//05190000072
char *variableList[1000]; int vIndex = 0; //declaration for every  line
int integerList[1000] = { 0 }; int iIndex = 0; //declaration for every int


bool isKeyword(char* str) { //all given identifiers,assignments and other keywords for analysis

	if (!strcmp(str, "int") ||
		!strcmp(str, "move") || !strcmp(str, "to") || !strcmp(str, "add") ||
		!strcmp(str, "sub") || !strcmp(str, "from") ||
		!strcmp(str, "loop") || !strcmp(str, "times") ||
		!strcmp(str, "out")) {
		return true;
	}

	return false;
}

bool isInteger(char* str) {
    if (!str || !*str)
        return false;
    int start = str[0] == '-' ? 1 : 0;
    if (!str[start])
        return false;
    for (int i = start; str[i]; i++) {
        if (!isdigit((unsigned char) str[i]))
            return false;
    }
    char *end;
    errno = 0;
    long value = strtol(str, &end, 10);
    return errno != ERANGE && *end == '\0' && value >= INT_MIN && value <= INT_MAX;
}

bool isIntegerVariable(char* str) { //check if integer assigned to any Identifier

	for (int i = 0; i < vIndex; i++) {
		if (!strcmp(str, variableList[i])) {
			return true;
		}
	}

	return false;
}

bool isVariable(char* str) //check if string is a Identifier declaration
{
	size_t len = strlen(str);

	if (len == 0 || len > 20) {
		return false;
	}

	for (size_t i = 0; i < len; i++) {
		if (!isalpha((unsigned char) str[0])) {
			return false;
		}
		if (!((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9') || (str[i] == '_'))) { //checking characters between A->Z ignoring Turkish chars
			return false;
		}
	}

	return true;
}

int isComment(char* str, FILE* fpout) {
    bool in_string = false;
    for (size_t i = 0; str[i]; i++) {
        if (str[i] == '"')
            in_string = !in_string;
        if (str[i] == '{' && !in_string) {
            char *end = strchr(str + i + 1, '}');
            if (!end) {
                fputs("Comment line is not completed.\n", fpout);
                return 1;
            }
            fputs("Comment Line: ", fpout);
            fwrite(str + i + 1, 1, (size_t) (end - str - i - 1), fpout);
            fputc('\n', fpout);
            str[i] = ' ';
            memmove(str + i + 1, end + 1, strlen(end + 1) + 1);
        }
    }
    return 0;
}

int parse(char* code,FILE* fpout) { //parsing to lines and splitting every line to words
    size_t code_length = strlen(code);
    while (code_length && isspace((unsigned char) code[code_length - 1]))
        code_length--;
    if (code_length && code[code_length - 1] != '.') {
        fputs("End of line is expected.\n", fpout);
        return 1;
    }
	int lineCount = 0;
	char *line[1000] = {0};
	char *splitCode = strtok(code, ".");
	while (splitCode != NULL) {
		if (lineCount >= 1000) {
            fputs("Statement limit exceeded.\n", fpout);
            return 1;
        }
        line[lineCount++] = splitCode;
		splitCode = strtok(NULL, ".");
	}

	for (int i = 0; i < lineCount; i++) {
		int wordCount = 0;
		char *word[1000] = {0};
		char *splitLine = strtok(line[i], " \t\n\r[]\"");
		while (splitLine != NULL) {
			if (wordCount >= 999) {
                fputs("Word limit exceeded.\n", fpout);
                return 1;
            }
            word[wordCount++] = splitLine;
			splitLine = strtok(NULL, " \t\n\r[]\"");
		}

		if (wordCount == 0) { continue; }

		//Integer check
		if (!strcmp(word[0], "int")) {
			fprintf(fpout, "%s", "'int' is a keyword.\n");

			//while checking integers Identifier check
			if (word[1] == NULL) {
				fprintf(fpout, "%s", "Identifier is expected.\n");
				return 1;
			}
			else if (isKeyword(word[1])) {
				fprintf(fpout, "%s %s", word[1]," is a keyword. Identifier is expected.\n");
				return 1;
			}
			else if (isVariable(word[1])) {
				fprintf(fpout, "%s %s", word[1] , " is an Identifier.\n");

				if (vIndex >= 1000 || isIntegerVariable(word[1])) {
                    fputs("Duplicate identifier or symbol limit exceeded.\n", fpout);
                    return 1;
                }
                char *name = malloc(strlen(word[1]) + 1);
                if (!name) {
                    fputs("Memory allocation failed.\n", fpout);
                    return 1;
                }
                strcpy(name, word[1]);
                variableList[vIndex++] = name;
				integerList[iIndex++] = 0;
			}
			else {;
				fprintf(fpout, "%s %s",word[1] ," is not an identifier. Identifier is expected.\n" );
				return 1;
			}

			//End of line
			if (wordCount == 2) {
				fprintf(fpout, "%s", "'.' End of Line.\n");
			}
			else {
				fprintf(fpout, "%s", "End of line is expected.\n");
				return 1;
			}
		}


		//Move check
		else if (!strcmp(word[0], "move")) {
			fprintf(fpout, "%s", "'move' is a keyword.\n");

			//what is moving ? -> integer or Identifier
			int moveInt = 0;
			if (word[1] == NULL) {
				fprintf(fpout, "%s", "Integer or identifier are expected.\n");
				return 1;
			}
			else if (isKeyword(word[1])) {
				fprintf(fpout, "%s %s", word[1], " is a keyword. Integer or identifier are expected.\n");
				return 1;
			}
			else if (isInteger(word[1])) {
                fprintf(fpout, "%s %s",word[1] ," is an integer.\n" );
				moveInt = (int) strtol(word[1], NULL, 10);
			}
			else if (isIntegerVariable(word[1])) {
				fprintf(fpout, "%s %s", word[1] ," is an identifier.\n");

				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[1])) {
						moveInt = integerList[i];
						break;
					}
				}
			}
			else {
				fprintf(fpout, "%s %s", word[1]," is not an integer or identifier. Integer or identifier are expected.\n" );
				return 1;
			}

			//move other side check -> To
			if (word[2] == NULL) {
				fprintf(fpout, "%s", "Keyword 'to' is expected.\n");
				return 1;
			}
			else if (!strcmp(word[2], "to")) {
				fprintf(fpout, "%s", "'to' is a keyword.\n");
			}
			else {
				fprintf(fpout, "%s", "Keyword 'to' is expected.\n");
				return 1;
			}

			//normal Identifier check
			if (word[3] == NULL) {
				fprintf(fpout, "%s", "Identifier is expected.\n");
				return 1;
			}
			else if (isKeyword(word[3])) {
				fprintf(fpout, "%s %s",word[3] , " is a keyword. Identifier is expected.\n");
				return 1;
			}
			else if (isIntegerVariable(word[3])) {
                fprintf(fpout, "%s %s",word[3] ," is an identifier.\n");
				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[3])) {
						integerList[i] = moveInt;
						break;
					}
				}
			}
			else {
				fprintf(fpout, "%s %s", word[3] , " is not an identifier. Identifier is expected.\n");
				return 1;
			}

			//End of line
			if (wordCount == 4) {
				fprintf(fpout, "%s", "'.' End of Line.\n");
			}
			else {
				fprintf(fpout, "%s", "End of line is expected.\n");
				return 1;
			}
		}


		//Addition operation check
		else if (!strcmp(word[0], "add")) {
			fprintf(fpout, "%s", "'add' is a keyword.\n");

			//Integer or Identifier
			int addInt = 0;
			if (word[1] == NULL) {
				fprintf(fpout, "%s", "Integer or identifier are expected.\n");
				return 1;
			}
			else if (isKeyword(word[1])) {
				fprintf(fpout, "%s %s", word[1]," is a keyword. Integer or identifier are expected.\n");
				return 1;
			}
			else if (isInteger(word[1])) {
				fprintf(fpout, "%s %s", word[1] ," is an integer.\n");

				addInt = (int) strtol(word[1], NULL, 10);
			}
			else if (isIntegerVariable(word[1])) {
                fprintf(fpout, "%s %s", word[1], " is an identifier.\n");
				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[1])) {
						addInt = integerList[i];
						break;
					}
				}
			}
			else {
				fprintf(fpout, "%s %s", word[1] ," is not an integer or identifier. Integer or identifier are expected.\n");
				return 1;
			}

			//addition other side check -> To
			if (word[2] == NULL) {
				fprintf(fpout, "%s", "Move is given and keyword 'to' is expected.\n");
				return 1;
			}
			else if (!strcmp(word[2], "to")) {
				fprintf(fpout, "%s", "'to' is a keyword.\n");
			}
			else {
				fprintf(fpout, "%s", "Move is given and keyword 'to' is expected.\n");
				return 1;
			}

			//Identifier check
			if (word[3] == NULL) {
				fprintf(fpout, "%s", "Identifier is expected.\n");
				return 1;
			}
			else if (isKeyword(word[3])) {
				fprintf(fpout, "%s %s", word[3] ," is a keyword. Identifier is expected.\n");
				return 1;
			}
			else if (isIntegerVariable(word[3])) {
				fprintf(fpout, "%s %s", word[3]," is an identifier.\n");

				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[3])) {
						if ((addInt > 0 && integerList[i] > INT_MAX - addInt) ||
                                (addInt < 0 && integerList[i] < INT_MIN - addInt)) {
                                fputs("Integer range exceeded.\n", fpout);
                                return 1;
                            }
                            integerList[i] += addInt;
						break;
					}
				}
			}
			else {
				fprintf(fpout, "%s %s",word[3]," is not an identifier. Identifier is expected.\n");
				return 1;
			}

			//End Of Line
			if (wordCount == 4) {
				fprintf(fpout, "%s", "'.' End of Line.\n");
			}
			else {
				fprintf(fpout, "%s", "End of line is expected.\n");
				return 1;
			}
		}


		//Subtraction operation check
		else if (!strcmp(word[0], "sub")) {
			fprintf(fpout, "%s", "'sub' is a keyword.\n");

			//Integer or Identifier
			int subInt = 0;
			if (word[1] == NULL) {
				fprintf(fpout, "%s", "Integer or Identifier are expected.\n");
				return 1;
			}
			else if (isKeyword(word[1])) {
				fprintf(fpout, "%s %s",word[1], " is a keyword. Integer or identifier are expected.\n");
				return 1;
			}
			else if (isInteger(word[1])) {
				fprintf(fpout, "%s %s",word[1], " is an integer.\n");

				subInt = (int) strtol(word[1], NULL, 10);
			}
			else if (isIntegerVariable(word[1])) {
				fprintf(fpout, "%s %s",word[1], " is an identifier.\n");

				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[1])) {
						subInt = integerList[i];
						break;
					}
				}
			}
			else {
				fprintf(fpout, "%s %s",word[1], " is not an integer or identifier. Integer or identifier are expected.\n");
				return 1;
			}

			//Substraction other side check-> To
			if (word[2] == NULL) {
				fprintf(fpout, "%s", "Keyword 'from' is expected.\n");
				return 1;
			}
			else if (!strcmp(word[2], "from")) {
				fprintf(fpout, "%s", "'from' is a keyword.\n");
			}
			else {
				fprintf(fpout, "%s", "Keyword 'from' is expected.\n");
				return 1;
			}

			//Identifier check
			if (word[3] == NULL) {
				fprintf(fpout, "%s", "Identifier is expected.\n");
				return 1;
			}
			else if (isKeyword(word[3])) {
				fprintf(fpout, "%s %s",word[3], " is a keyword. Identifier is expected.\n");
				return 1;
			}
			else if (isIntegerVariable(word[3])) {
                fprintf(fpout, "%s %s",word[3]," is an identifier.\n");
				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[3])) {
						if ((subInt < 0 && integerList[i] > INT_MAX + subInt) ||
                                (subInt > 0 && integerList[i] < INT_MIN + subInt)) {
                                fputs("Integer range exceeded.\n", fpout);
                                return 1;
                            }
                            integerList[i] -= subInt;
						break;
					}
				}
			}
			else {
				fprintf(fpout, "%s %s",word[3], " is not an identifier. Identifier is expected.\n");
				return 1;
			}

			//End of line
			if (wordCount == 4) {
				fprintf(fpout, "%s", "'.' End of Line.\n");
			}
			else {
				fprintf(fpout, "%s", "End of line is expected.\n");
				return 1;
			}
		}

		//string output check -> Out
		else if (!strcmp(word[0], "out")) {
            fprintf(fpout, "%s", "'out' is a keyword.\n");
			//Integer, Variable or String
			if (word[1] == NULL) {
				fprintf(fpout, "%s", "Integer, identifier or string are expected.\n");
				return 1;
			}
			else if (isInteger(word[1])) {
				fprintf(fpout, "%s %s",word[1] ," is an integer.\n");
				fprintf(fpout, "%s %s %s", "Integer Value: ",word[1],"\n");
			}
			else if (isIntegerVariable(word[1])) {
				fprintf(fpout, "%s %s",word[1], " is an identifier.\n");

				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[1])) {
						fprintf(fpout, "%s %s %d %s","Identifier value: ",word[1],integerList[i],"\n");
						break;
					}
				}
			}

            else {
                fputs("An integer or declared identifier is expected for output.\n", fpout);
                return 1;
            }

			//End of line
			if (wordCount == 2) {
				fprintf(fpout, "%s","'.' End of Line.\n");
			}
			else {
				fprintf(fpout, "%s","End of line is expected.\n");
				return 1;
			}
		}

		//Loop check
		else if (!strcmp(word[0], "loop")) {
            fprintf(fpout, "%s","'loop' is a keyword.\n");
			//Integer or Identifier
			int loopInt = 0;
			if (word[1] == NULL) {
				fprintf(fpout, "%s","Integer or identifier are expected.\n");
				return 1;
			}
			else if (isKeyword(word[1])) {
				fprintf(fpout, "%s %s",word[1], " is a keyword. Integer or identifier are expected.\n");
				return 1;
			}
			else if (isInteger(word[1])) {
				fprintf(fpout, "%s %s",word[1] ," is an integer.\n");
				loopInt = (int) strtol(word[1], NULL, 10);
			}
			else if (isIntegerVariable(word[1])) {
				fprintf(fpout, "%s %s",word[1], " is an identifier.\n");

				for (int i = 0; i < vIndex; i++) {
					if (!strcmp(variableList[i], word[1])) {
						loopInt = integerList[i];
						break;
					}
				}
			}
			else {
				fprintf(fpout, "%s %s %s", "Keyword 'times' is expected. ",word[1],"\n");
				return 1;
			}

			//how many times loop continiues
			if (word[2] == NULL) {
				fprintf(fpout, "%s", "Keyword 'times' is expected.\n");
				return 1;
			}
			else if (!strcmp(word[2], "times")) {
				fprintf(fpout, "%s", "'times' is a keyword.\n");
			}
			else {
				fprintf(fpout, "%s", "Keyword 'times' is expected.\n");
				return 1;
			}

            if (loopInt <= 0 || wordCount <= 3) {
                fputs("Positive loop count and loop body are expected.\n", fpout);
                return 1;
            }
            char body[100] = "";
            size_t body_length = 0;
            for (int j = 3; j < wordCount; j++) {
                size_t length = strlen(word[j]);
                if (body_length + length + 3 >= sizeof(body)) {
                    fputs("Loop body length limit exceeded.\n", fpout);
                    return 1;
                }
                body[body_length++] = ' ';
                memcpy(body + body_length, word[j], length);
                body_length += length;
                body[body_length] = '\0';
            }
            body[body_length++] = '.';
            body[body_length] = '\0';
            for (int j = 0; j < loopInt; j++) {
                char newCode[100];
                memcpy(newCode, body, body_length + 1);
                if (parse(newCode, fpout))
                    return 1;
            }

			//End of line
			if (wordCount > 4) {
				fprintf(fpout, "%s", "End of Line.\n");
			}
			else {
				fprintf(fpout, "%s", "End of line is expected.\n");

				return 1;
			}
		}

		else {

			fprintf(fpout, "Unsupported statement: %s\n", word[0]);
            return 1;
		}
	}
    return 0;
}

int main(int argc, char *argv[]) {
    char fileName[1024];
    if (argc > 2) {
        fputs("Expected at most one filename argument.\n", stderr);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        if (strlen(argv[1]) >= sizeof(fileName)) {
            fputs("Filename is too long.\n", stderr);
            return EXIT_FAILURE;
        }
        strcpy(fileName, argv[1]);
    } else {
        printf("Enter a .ba filename: ");
        if (!fgets(fileName, sizeof(fileName), stdin))
            return EXIT_FAILURE;
        size_t length = strcspn(fileName, "\r\n");
        if (fileName[length] == '\0' && !feof(stdin)) {
            fputs("Filename is too long.\n", stderr);
            return EXIT_FAILURE;
        }
        fileName[length] = '\0';
    }
    size_t length = strlen(fileName);
    if (length < 3 || strcmp(fileName + length - 3, ".ba") != 0) {
        fputs("File extension must be '.ba'.\n", stderr);
        return EXIT_FAILURE;
    }
    FILE *fp = fopen(fileName, "rb");
    if (!fp) {
        perror(fileName);
        return EXIT_FAILURE;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return EXIT_FAILURE;
    }
    long file_size = ftell(fp);
    if (file_size < 0 || file_size > 1048576 || fseek(fp, 0, SEEK_SET) != 0) {
        fputs("Input exceeds the 1 MiB limit or could not be read.\n", stderr);
        fclose(fp);
        return EXIT_FAILURE;
    }
    char *code = calloc((size_t) file_size + 1, 1);
    if (!code) {
        fclose(fp);
        return EXIT_FAILURE;
    }
    if (fread(code, 1, (size_t) file_size, fp) != (size_t) file_size || ferror(fp)) {
        fputs("Source read failed.\n", stderr);
        fclose(fp);
        free(code);
        return EXIT_FAILURE;
    }
    fclose(fp);
    if (memchr(code, '\0', (size_t) file_size)) {
        fputs("Input contains a NUL byte.\n", stderr);
        free(code);
        return EXIT_FAILURE;
    }
    FILE *fpout = fopen("myscript.lx", "w");
    if (!fpout) {
        perror("myscript.lx");
        free(code);
        return EXIT_FAILURE;
    }
    int result = isComment(code, fpout);
    if (!result)
        result = parse(code, fpout);
    if (ferror(fpout))
        result = 1;
    if (fclose(fpout) != 0)
        result = 1;
    for (int i = 0; i < vIndex; i++)
        free(variableList[i]);
    free(code);
    printf("Analysis written to myscript.lx.\n");
    return result ? EXIT_FAILURE : EXIT_SUCCESS;
}
