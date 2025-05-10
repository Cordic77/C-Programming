##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## so-utf8-x64-release
ProjectName            :=mandelbrot
ConfigurationName      :=so-utf8-x64-release
WorkspacePath          :=/drv/d/Projects/C/Mandelbrot/projects
ProjectPath            :=/drv/d/Projects/C/Mandelbrot/projects
IntermediateDirectory  :=../bin/so-utf8-x64-release/obj/codelite
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=scott
Date                   :=08/01/19
CodeLitePath           :=/home/scott/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/../../$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)_GNU_SOURCE=1 $(PreprocessorSwitch)_REENTRANT $(PreprocessorSwitch)NDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="mandelbrot.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            := $(IntermediateDirectory)/asmlib.o -s -m64
IncludePath            :=  $(IncludeSwitch)../include $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)pthread $(LibrarySwitch)SDL $(LibrarySwitch)SDL_ttf 
ArLibs                 :=  "pthread" "SDL" "SDL_ttf" 
LibPath                := $(LibraryPathSwitch)$(IntermediateDirectory) 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS := -Wmain -pedantic-errors -ansi -pedantic -W -Wall -Wextra -Werror -m64 -O2 -Wall $(Preprocessors)
CFLAGS   := -Wmain -pedantic-errors -ansi -pedantic -W -Wall -march=native -Wextra -Werror -m64 -O2 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/up_src_Complex_ADT.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_FloatPoint.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_main.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_Graphics.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_DrawFractal.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_Fractal.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_Fractal_SIMD.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ../bin/so-utf8-x64-release/obj/codelite || $(MakeDirCommand) ../bin/so-utf8-x64-release/obj/codelite


$(IntermediateDirectory)/.d:
	@test -d ../bin/so-utf8-x64-release/obj/codelite || $(MakeDirCommand) ../bin/so-utf8-x64-release/obj/codelite
PrePreBuild: $(IntermediateDirectory)/asmlib.o
$(IntermediateDirectory)/asmlib.o:
	mkdir -p '$(IntermediateDirectory)'
	nasm -dLIBRARY -f elf64 -Xgnu -o '$(IntermediateDirectory)/asmlib.o' ../src/asmlib.asm

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_src_Complex_ADT.c$(ObjectSuffix): ../src/Complex_ADT.c $(IntermediateDirectory)/up_src_Complex_ADT.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/drv/d/Projects/C/Mandelbrot/src/Complex_ADT.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_Complex_ADT.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_Complex_ADT.c$(DependSuffix): ../src/Complex_ADT.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_Complex_ADT.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_Complex_ADT.c$(DependSuffix) -MM ../src/Complex_ADT.c

$(IntermediateDirectory)/up_src_Complex_ADT.c$(PreprocessSuffix): ../src/Complex_ADT.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_Complex_ADT.c$(PreprocessSuffix) ../src/Complex_ADT.c

$(IntermediateDirectory)/up_src_FloatPoint.c$(ObjectSuffix): ../src/FloatPoint.c $(IntermediateDirectory)/up_src_FloatPoint.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/drv/d/Projects/C/Mandelbrot/src/FloatPoint.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_FloatPoint.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_FloatPoint.c$(DependSuffix): ../src/FloatPoint.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_FloatPoint.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_FloatPoint.c$(DependSuffix) -MM ../src/FloatPoint.c

$(IntermediateDirectory)/up_src_FloatPoint.c$(PreprocessSuffix): ../src/FloatPoint.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_FloatPoint.c$(PreprocessSuffix) ../src/FloatPoint.c

$(IntermediateDirectory)/up_src_main.c$(ObjectSuffix): ../src/main.c $(IntermediateDirectory)/up_src_main.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/drv/d/Projects/C/Mandelbrot/src/main.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_main.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_main.c$(DependSuffix): ../src/main.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_main.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_main.c$(DependSuffix) -MM ../src/main.c

$(IntermediateDirectory)/up_src_main.c$(PreprocessSuffix): ../src/main.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_main.c$(PreprocessSuffix) ../src/main.c

$(IntermediateDirectory)/up_src_Graphics.c$(ObjectSuffix): ../src/Graphics.c $(IntermediateDirectory)/up_src_Graphics.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/drv/d/Projects/C/Mandelbrot/src/Graphics.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_Graphics.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_Graphics.c$(DependSuffix): ../src/Graphics.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_Graphics.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_Graphics.c$(DependSuffix) -MM ../src/Graphics.c

$(IntermediateDirectory)/up_src_Graphics.c$(PreprocessSuffix): ../src/Graphics.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_Graphics.c$(PreprocessSuffix) ../src/Graphics.c

$(IntermediateDirectory)/up_src_DrawFractal.c$(ObjectSuffix): ../src/DrawFractal.c $(IntermediateDirectory)/up_src_DrawFractal.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/drv/d/Projects/C/Mandelbrot/src/DrawFractal.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_DrawFractal.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_DrawFractal.c$(DependSuffix): ../src/DrawFractal.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_DrawFractal.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_DrawFractal.c$(DependSuffix) -MM ../src/DrawFractal.c

$(IntermediateDirectory)/up_src_DrawFractal.c$(PreprocessSuffix): ../src/DrawFractal.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_DrawFractal.c$(PreprocessSuffix) ../src/DrawFractal.c

$(IntermediateDirectory)/up_src_Fractal.c$(ObjectSuffix): ../src/Fractal.c $(IntermediateDirectory)/up_src_Fractal.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/drv/d/Projects/C/Mandelbrot/src/Fractal.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_Fractal.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_Fractal.c$(DependSuffix): ../src/Fractal.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_Fractal.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_Fractal.c$(DependSuffix) -MM ../src/Fractal.c

$(IntermediateDirectory)/up_src_Fractal.c$(PreprocessSuffix): ../src/Fractal.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_Fractal.c$(PreprocessSuffix) ../src/Fractal.c

$(IntermediateDirectory)/up_src_Fractal_SIMD.c$(ObjectSuffix): ../src/Fractal_SIMD.c $(IntermediateDirectory)/up_src_Fractal_SIMD.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/drv/d/Projects/C/Mandelbrot/src/Fractal_SIMD.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_Fractal_SIMD.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_Fractal_SIMD.c$(DependSuffix): ../src/Fractal_SIMD.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_Fractal_SIMD.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_Fractal_SIMD.c$(DependSuffix) -MM ../src/Fractal_SIMD.c

$(IntermediateDirectory)/up_src_Fractal_SIMD.c$(PreprocessSuffix): ../src/Fractal_SIMD.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_Fractal_SIMD.c$(PreprocessSuffix) ../src/Fractal_SIMD.c


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ../bin/so-utf8-x64-release/obj/codelite/


