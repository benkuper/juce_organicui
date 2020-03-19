/*
 ==============================================================================

 StringUtil.h
 Created: 8 Mar 2016 2:43:25pm
 Author:  bkupe

 ==============================================================================
 */

#pragma once

class CommandLineElement{
public:
  CommandLineElement(const String & name = ""):command(name){}
  String command;
  StringArray args;

  bool isEmpty(){return (command == "") && (args.size()==0);}
  CommandLineElement & operator=(const CommandLineElement & rhs)
  {
    if(this == &rhs)
      return *this;
    command = rhs.command;
    args = rhs.args;
    return *this;
  }
  operator bool(){
    return !isEmpty();
  }

};

//typedef Array<CommandLineElement> CommandLineElements;
class CommandLineElements: public Array<CommandLineElement>{
public:

  CommandLineElement  getCommandLineElement(const String command){
    for(auto & e:*this ){
      if(e.command == command){
        return e;
      }
    }
    return CommandLineElement();
  }

  CommandLineElement  getCommandLineElement(const String command, const String command2 ){
    for(auto & e:*this ){
      if((e.command == command) || (e.command==command2)){
        return e;
      }
    }
    return CommandLineElement();
  }

};

class StringUtil
{
public:
	static String toShortName(const String& niceName, bool replaceSlashes = false);
	static CommandLineElements parseCommandLine(const String& commandLine);
};
