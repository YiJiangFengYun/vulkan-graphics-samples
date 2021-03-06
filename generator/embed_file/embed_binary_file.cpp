#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <boost/filesystem.hpp>

FILE* open_or_exit(const char* path, const char* mode)
{
    auto boostPath = boost::filesystem::path(path);
    boost::filesystem::create_directory(boostPath.parent_path());
    FILE* fp = fopen(path, mode);
    if ( !fp ) {
        auto errStr = std::string("failed to open file ") + std::string(path);
        perror(errStr.c_str());
        exit(1);
    }
    return fp;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Please specify variable name to save embed value in code, input file to embed and out file to create code.");
        return 4;
    }    
    const char * variableName = argv[1];
    const char * inputFilePath = argv[2];
    const char * outputFilePath = argv[3];
    FILE* in = open_or_exit(inputFilePath, "rb");    
    FILE* out = open_or_exit(outputFilePath, "w");   
    size_t lSize;
    unsigned char * buffer;
    size_t result;
    fseek(in, 0, SEEK_END);
    lSize = static_cast<size_t>(ftell(in));
    rewind(in);    
    // allocate memory to contain the whole file:
    buffer = (unsigned char*)malloc(sizeof(char)*lSize);
    if (buffer == NULL) 
    {
      fprintf(stderr, "Memory aloocate error");
      exit(2); 
    }    
    // copy the file into the buffer:
    result = fread(buffer, 1, lSize, in);
    if (result != lSize) 
    { 
      fprintf(stderr, "Reading file error, target size: %d, result size: %d", lSize, result);
      fputs("Reading error", stderr); 
      exit(3); 
    }    
    /* the whole file is now loaded in the memory buffer. */
    size_t linecount = 0;
    size_t i;
        fprintf(out, "#include <stdint.h>\n");
    fprintf(out, "const unsigned char %s[] = {\n", variableName);
    for (i = 0; i < lSize; ++i) {
        fprintf(out, "0x%02x, ", *(buffer + i));
        if (++linecount == 10) { fprintf(out, "\n"); linecount = 0; }
    }
    if (linecount > 0) fprintf(out, "\n");
    fprintf(out, "};\n");
    fprintf(out, "const size_t %s_LEN = sizeof(%s);\n\n", variableName, variableName);    
    fclose(in);
    fclose(out);
    free(buffer);
    return EXIT_SUCCESS;
}