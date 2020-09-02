#ifndef _XML_H
#define _XML_H

#include <stdarg.h> //for va_list
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
//#define WAITNL {printf("Press enter..."); scanf("%*c");}
#define WAITNL {}

class XMLReader
{
	static std::string GetFolder(std::string file)
	{
		size_t found = file.find_last_of("/\\");
		if( found == std::string::npos )
			return "";
		else return file.substr(0,found);
	}
	struct Stream
	{
		std::string src;
		std::istream * input;
		int linebreak, linechar;
		int hadlinebreak, hadlinechar;
	};
	std::vector<Stream> inp;
	int verbose;
	enum State
	{
		Intro, //read tag or read tag contents
		WaitTag, //wait tag name or comment
		ReadTag, //reading in tag name
		ReadCommentExclamation, //skipping comments
		ReadCommentQuestion, //skipping comments
		WaitAttribute, //reading attribute name
		ReadAttribute,
		WaitAttributeValue, //read attribute value
		ReadAttributeValue,
		ReadAttributeValueQuote,
		EndTag, //tag ended read closing
		WaitCloseTag,
		ReadCloseTagSlash,
		ReadCloseTagName,
		EndOfFile, // end of file reached
		Failure //unexpected error
	} _state;
	Stream & GetStream() {return inp.back();}
	const Stream & GetStream() const {return inp.back();}
	std::istream & GetInputStream() {return *inp.back().input;}
	const std::istream & GetInputStream() const {return *inp.back().input;}
	//should not share the reference to the stream with another reader
	XMLReader(const XMLReader & other) {}
	XMLReader & operator =(XMLReader & other) {return *this;}
	char GetChar()
	{
		char c = '\0';
		GetInputStream().get(c);
		GetStream().hadlinebreak = GetStream().linebreak;
		GetStream().hadlinechar = GetStream().linechar;
		if( c == '\n' ) 
		{
			++GetStream().linebreak;
			GetStream().linechar = 0;
		}
		else ++GetStream().linechar;
		if( GetInputStream().eof() ) 
		{
			if( inp.size() > 1 )
			{
				PopStream();
				c = GetChar();
			}
			else
				_state = EndOfFile;
		}
		if( GetInputStream().fail() )
		{
			Report("Stream failed while getting the char, state %s",StateName(_state).c_str());
			WAITNL;
			_state = Failure;
		}
		return c;
	}
	//return one character back to the stream
	void RetChar()
	{
		GetStream().linebreak = GetStream().hadlinebreak;
		GetStream().linechar = GetStream().hadlinechar;
		GetInputStream().unget();
		if( GetInputStream().fail() ) 
		{
			Report("Stream failed while ungetting the char");
			WAITNL;
			_state = Failure;
		}
	}
	void SkipComments(State RetState)
    {
		int ntmp = 0;
		char tmp[3] = {'\0','\0','\0'};
		char c;
		bool done = false;
		if( _state == ReadCommentExclamation )
		{
			while(!done)
			{
				c = GetChar();
				if( _state == Failure )
				{
					Report("Unexpected failure while skipping comments");
					done = true;
				}
				else if( _state == EndOfFile )
				{
					Report("Unexpected end of file while skipping comments");
					done = true;
				}
				tmp[ntmp] = c;
				if( tmp[ntmp] == '>' && tmp[(ntmp-1+3)%3] == '-' && tmp[(ntmp-2+3)%3] == '-' ) 
				{
					_state = RetState;
					done = true;
				}
				ntmp = (ntmp+1)%3;
			}
		}
		else if( _state == ReadCommentQuestion )
		{
			while(!done)
			{
				c = GetChar();
				if( _state == Failure )
				{
					Report("Unexpected failure while skipping comments");
					done = true;
				}
				else if( _state == EndOfFile )
				{
					Report("Unexpected end of file while skipping comments");
					done = true;
				}
				tmp[ntmp] = c;
				if( tmp[ntmp] == '>' && tmp[(ntmp-1+2)%2] == '?' ) 
				{
					_state = RetState;
					done = true;
				}
				ntmp = (ntmp+1)%2;
			}
		}
		else 
		{
			Report("Unexpected state %s while reading comments",StateName(_state).c_str());
			_state = Failure; //What are we doing here?
		}
	}
    std::string StateName(State s) const
    {
		switch(s)
		{
		case Intro: return "Intro";
		case WaitTag: return "WaitTag";
		case ReadTag: return "ReadTag";
		case ReadCommentExclamation: return "ReadCommentExclamation";
		case ReadCommentQuestion: return "ReadCommentQuestion";
		case WaitAttribute: return "WaitAttribute";
		case ReadAttribute: return "ReadAttribute";
		case WaitAttributeValue: return "WaitAttributeValue";
		case ReadAttributeValue: return "ReadAttributeValue";
		case ReadAttributeValueQuote: return "ReadAttributeValueQuote";
		case EndTag: return "EndTag";
		case WaitCloseTag: return "WaitCloseTag";
		case ReadCloseTagSlash: return "WaitCloseTagSlash";
		case ReadCloseTagName: return "WaitCloseTagName";
		case EndOfFile: return "EndOfFile";
		case Failure: return "Failure";
		};
		return "Unspecified";
	}
  public:
	/// 2 - lot's of output
	/// 1 - output key steps, currently in ReadXML
	void SetVerbosity(int verbosity) {verbose = verbosity;}
	void Report(const char * fmt, ...) const
	{ 
		std::cout << GetStream().src << ":row:" << GetStream().linebreak << ":col:" << GetStream().linechar << " ";
		{
			char stext[16384];
			va_list ap;
			if ( fmt == NULL ) {std::cout << std::endl; return;}
			va_start(ap,fmt);
			vsprintf(stext,fmt,ap);
			va_end(ap);
			std::cout << stext;
		}
		std::cout << std::endl;
	}
	XMLReader(std::string sourcename, std::istream & input) : _state(Intro)
	{
		Stream add;
		add.src = sourcename;
		add.linebreak = 0;
		add.linechar = 0;
		add.hadlinebreak = 0;
		add.hadlinechar = 0;
		add.input = &input;
		inp.push_back(add); 
		verbose = 0;
		if( GetInputStream().fail() )
			Report("Got a bad stream on input in %s",__FUNCTION__);
	}
    void PushStream(std::string file)
    {
		Stream add;
		add.linebreak = 0;
		add.linechar = 0;
		add.hadlinebreak = 0;
		add.hadlinechar = 0;
		add.src = file;
		add.input = new std::fstream(file.c_str(),std::ios::in);

        if (!add.input->fail()) {
            inp.push_back(add);
        } else {
            Report("Got a bad stream on input in %s (Include stream %s)" ,__FUNCTION__, file.c_str());
            delete add.input;
        }

		if( GetInputStream().fail() )
		{
			Report("Got a bad stream on input in %s",__FUNCTION__);
		}
	}
    void PopStream()
    {
		if( inp.size() > 1 )
			delete static_cast<std::fstream *>(inp.back().input);
		inp.pop_back();
		if( _state == EndOfFile && !inp.empty() )
			_state = Intro;
	}
	//wait for '<' on input,
	//returns true and changes state to WaitTag if '<' encountered,
	//otherwise returns false
	bool ExpectOpenTag()
	{
		char c;
		bool done = false;
		if( !(_state == Intro) )
		{
			Report("Cannot expect open tag from state %s",StateName(_state).c_str());
			_state = Failure;
			return "";
		}
		while(!done)
		{
			c = GetChar(); 
			switch(_state)
			{
			case Intro:
				if( c == '<' )
				{
					if( verbose > 1 ) Report("info: encountered expected '<' symbol");
					_state = WaitTag;
					return true;
				}
				else if(!isspace(c))
				{
					if( verbose > 1 ) Report("info: encountered %x instead of expected '<'(%x) symbol",c,'<');
					RetChar();
					return false;
				}
				break;
			case EndOfFile: Report("Unexpected end of file while reading XML tag name"); done = true; break;
			case Failure: Report("Unrecoverable error while reading XML tag name"); done = true; break;
			default: Report("Unexpected state %s",StateName(_state).c_str()); done = true; break;
			}
		}
		return false;
	}
    //read in <TagName returns TagName
    std::string ReadOpenTag()
    {
		std::string ret;
		char c;
		bool done = false;
		if( !(_state == Intro || _state == WaitTag) )
		{
			Report("Cannot open tag from state %s",StateName(_state).c_str());
			_state = Failure;
			return "";
		}
		while(!done)
		{
			c = GetChar(); 
			switch(_state)
			{
			case Intro:
				if( c == '<' ) 
				{
					if( verbose > 1 ) Report("info: waiting tag name");
					_state = WaitTag;
				}
				else if( !isspace(c) ) //do not expect anything except for spacing
				{
					Report("Unexpected text character %c",c);
					_state = Failure;
					done = true;
				}
				break;
			case WaitTag:
				if( c == '?' ) 
				{
					if( verbose > 1 ) Report("info: skipping comments");
					_state = ReadCommentQuestion;
					SkipComments(WaitTag);
					_state = Intro; //wait for '<' again
				}
				else if( c == '!' ) //can be ![CDATA[
				{
					c = GetChar(); //check next character
					if( c == '-' ) //this is going to be comment
					{
						c = GetChar(); //check next character
						if( c == '-' )
						{
							if( verbose > 1 ) Report("info: skipping comments");
							_state = ReadCommentExclamation;
							SkipComments(WaitTag);
							_state = Intro; //wait for '<' again
						}
						else Report("unexpected character %c while reading comment",c);
					}
					else if( c == '[' ) // this is ![CDATA[
					{
						if( verbose > 1 ) Report("info: reading ![CDATA[");
						ret.push_back('!');
						ret.push_back(c);
						_state = ReadTag;
					}
					else Report("unexpected character %c while reading comment or ![CDATA[ block",c);
				}
				else if( c == '/' )
				{
					if( verbose > 1 ) Report("info: encountered closing slash");
					RetChar();
					_state = ReadCloseTagSlash;
					done = true;
				}
				else if( isalpha(c) || c == '_' )
				{
					if( verbose > 1 ) Report("info: reading tag name");
					ret.push_back(c);
					_state = ReadTag;
				}
				break;
			case ReadTag:
				if( isspace(c) ) 
				{
					if( verbose > 1 ) Report("info: waiting attribute name");
					done = true;
					_state = WaitAttribute;
				}
				else if( c == '/' || c == '>' )
				{
					if( verbose > 1 ) Report("info: tag ended");
					RetChar(); //push character back to the stream
					done = true;
					_state = EndTag;
				}
				else if( isalpha(c) || (!ret.empty() && isprint(c)) )
				{
					ret.push_back(c);
					if( ret == "![CDATA[" )
					{
						done = true;
						_state = EndTag;
					}
				}
				else Report("unexpected character %c in XML tag name",c);
				break;
			case EndOfFile: Report("Unexpected end of file while reading XML tag name"); done = true; break;
			case Failure: Report("Unrecoverable error while reading XML tag name"); done = true; break;
			default: Report("Unexpected state %s",StateName(_state).c_str()); done = true; break;
			}
		}
		if( verbose > 1 ) Report("info: opened tag %s",ret.c_str());
		return ret;
	}
	//read > or /> skipping for attributes
	int ReadCloseTag()
	{
		char tmp[2];
		tmp[0] = GetChar();
		if( tmp[0] == '>' )
		{
			_state = Intro;
			if( verbose > 1 ) Report("info: closed tag");
			return 1; //tag was finished with >
		}
		else if( tmp[0] == '/' ) //close single stage tag
		{
			tmp[1] = GetChar();
			if( tmp[1] == '>' )
			{
				_state = Intro;
				if( verbose > 1 ) Report("info: closed tag");
				return 2; //tag was halted with />
			}
			Report("Encountered %x%x while expecting '/>'(%x%x) for tag closing",tmp[0],tmp[1],'/','>');
		}
		Report("Encountered %x while expecting '>'(%x) for tag closing",tmp[0],'>');
		_state = Failure;
		return 0;
	}
    bool isTagFinish() const {return _state == ReadCloseTagSlash;}
    //read </TagName> or fail
    bool ReadFinishTag(std::string TagName)
    {
		std::string name;
		bool done = false;
		char c;
		if( !(_state == Intro || _state == ReadCloseTagSlash) )
		{
			Report("Cannot read finish tag from state %s",StateName(_state).c_str());
			_state = Failure;
			return false;
		}
		if( _state == Intro ) _state = WaitCloseTag;
		while(!done)
		{
			c = GetChar();  
			switch(_state)
			{
			case WaitCloseTag:
				if( isspace(c) ) continue;
				if( c != '<' )
				{
					Report("Expected '<' instead of %c",c);
					_state = Failure;
					return false;
				}
				else _state = ReadCloseTagSlash;
				break;
			case ReadCloseTagSlash:
				if( c == '?' )
				{
					if( verbose > 1 ) Report("info: skipping comments");
					_state = ReadCommentQuestion;
					SkipComments(WaitCloseTag);
				}
				else if( c == '!' )
				{
					if( verbose > 1 ) Report("info: skipping comments");
					_state = ReadCommentExclamation;
					SkipComments(WaitCloseTag);
				}
				else if( c != '/' )
				{
					Report("Expected '/' instead of %c",c);
					_state = Failure;
					return false;
				}
				else _state = ReadCloseTagName;
				break;
			case ReadCloseTagName:
				if( c == '>' ) done = true;
				else if( isalpha(c) || (!name.empty() && isprint(c)) ) name.push_back(c);
				else Report("Unexpected symbol %c in tag name",c);
				break;
			case EndOfFile:
				Report("Unexpected end of file while searching for </%s>",TagName.c_str());
				return false;
			case Failure:
				Report("Unexpected failure while searching for </%s>",TagName.c_str());
				return false;
			default: Report("Unexpected state %s",StateName(_state).c_str()); return false;

			}
		}
		if( verbose > 1 ) Report("info: finished tag %s",name.c_str());
		_state = Intro;
		return name == TagName;
	}
    //read next attribute name, check isTagEnded
    std::string AttributeName()
    {
		std::string ret;
		bool done = false;
		char c;
		if( _state == EndTag ) return "";
		if( _state != WaitAttribute )
		{
			Report("Attribute name was not expected, state %s",StateName(_state).c_str());
			done = true;
		}
		while(!done)
		{
			c = GetChar();
			switch(_state)
			{
			case WaitAttribute:
				if( isspace(c) ) continue;
				else if( c == '<' )
				{
					c = GetChar();
					if( c == '?' )
					{
						if( verbose > 1 ) Report("info: skipping comments");
						_state = ReadCommentQuestion;
						SkipComments(WaitAttribute);
					}
					else if( c == '!' )
					{
						if( verbose > 1 ) Report("info: skipping comments");
						_state = ReadCommentExclamation;
						SkipComments(WaitAttribute);
					}
					else 
					{
						Report("Expected a comment, got '<%c'",c);
						_state = Failure;
						done = true;
					}
				}
				else if( isalpha(c) || c == '_' ) 
				{
					if( verbose > 1 ) Report("info: reading attribute name");
					ret.push_back(c);
					_state = ReadAttribute;
				}
				else if( c == '>' || c == '/' )
				{
					if( verbose > 1 ) Report("info: tag ended");
					RetChar();
					done = true;
					_state = EndTag;
				}
				break;
			case ReadAttribute:
				if( c == '=' || isspace(c) ) 
				{
					if( c == '=' ) RetChar();
					_state = WaitAttributeValue;
					done = true;
				}
				else if (isalpha(c) || isdigit(c) || c == '_' || c == '-' || c == '.' || c == ':') ret.push_back(c);
				else 
				{
					Report("Unexpected symbol %c while reading attribute name",c);
					_state = Failure;
					done = true;
				}
				break;
			case EndOfFile:
				Report("Unexpected end of file while reading attribute name");
				done = true;
				break;
			case Failure:
				Report("Unexpected failure while reading attribute name");
				done = true;
				break;
			default: Report("Unexpected state %s",StateName(_state).c_str()); done = true; break;
			}
		}
		if( verbose > 1 ) Report("info: attribute name %s",ret.c_str());
		return ret;
	}
    //read value of the attribute after reading it's name
    std::string AttributeValue()
    {
		std::string ret;
		bool done = false;
		char c;
		if( _state == EndTag ) return "";
		if( _state != WaitAttributeValue )
		{
			Report("Attribute value was not expected, state %s",StateName(_state).c_str());
			done = true;
		}
		while(!done)
		{
			c = GetChar();
			switch(_state)
			{
			case WaitAttributeValue:
				if( isspace(c) ) continue;
				else if( c == '=' )
				{
					if( verbose > 1 ) Report("info: reading attribute value");
					_state = ReadAttributeValue;
				}
				else if( c == '>' || c == '/' )
					Report("Unexpected end of XML tag while searching for '='");
				else Report("Unexpected character %c while searching for '='",c);
				break;
			case ReadAttributeValue:
				if( isspace(c) && ret.empty() ) continue;
				else if( c == '"' && ret.empty() ) 
				{
					if( verbose > 1 ) Report("info: reading attribute value in quotes");
					_state = ReadAttributeValueQuote;
				}
				else if( c == '>' || c =='/' )
				{
					if( verbose > 1 ) Report("info: end of tag");
					_state = EndTag;
					done = true;
				}
				else if( !isspace(c) ) ret.push_back(c);
				else if( isspace(c) )
				{
					if( verbose > 1 ) Report("info: end reading attribute value");
					_state = WaitAttribute;
					done = true;
				}
				else Report("Unexpected symbol %c while reading attribute value",c);
				break;
			case ReadAttributeValueQuote:
				if( c == '"' )
				{
					if( verbose > 1 ) Report("info: end reading attribute value");
					_state = WaitAttribute;
					done = true;
				}
				else if( !isprint(c) )
				{
					Report("Unprintable character %x encountered",c);
					_state = Failure;
					done = true;
				}
				else ret.push_back(c);
				break;
			case EndOfFile:
				Report("Unexpected end of file while reading attribute name");
				done = true;
				break;
			case Failure:
				Report("Unexpected failure while reading attribute name");
				done = true;
				break;
			default: Report("Unexpected state %s",StateName(_state).c_str()); done = true; break;
			}
		}
		if( verbose > 1 ) Report("info: attribute value %s",ret.c_str());
		return ret;
	}
    // > or /> was reached, should close ReadCloseTag
    // to finalize
    bool isTagEnded() const {return _state == EndTag;}
    bool isFailure() const {return _state == Failure;}
    bool isEof() const {return _state == EndOfFile;}    
	/// Structure for xml attribute.
    struct XMLAttrib
    {
      std::string name; //< Name of the attribute.
      std::string value; //< Value of the attribute.
    };
	/// Structure for xml tag with attributes.
    struct XMLTag
    {
      std::string name; //<Name of the XML tag.
      std::vector<XMLAttrib> attributes; //<List of attributes.
      int finish; //<Whether to close the tag.
	  ///This is data without ![CDATA[ wrap.
	  bool RawData() const {return finish == 5;}
	  ///This is data within ![CDATA[ wrap.
	  bool BlockData() const {return finish == 4;}
      ///Was not able to read the tag.
      bool Failure() const {return finish == 0;}
      ///Tag was red and have internal contents, can process the contents.
      bool Process() const {return finish == 1;}
	  ///Tag was red but do not have internal contents.
	  bool Stub() const {return finish == 2;}
      ///Tag was not red, finish of enclosing tag was encountered.
      bool Finalize() const {return finish == 3;}
	  ///Retrive attribute number n.
      const XMLAttrib & GetAttrib(int n) const {return attributes[n];}
	  ///Retrive attribute number n.
      XMLAttrib & GetAttrib(int n) {return attributes[n];}
	  ///Retrive number of attributes.
      int NumAttrib() const {return (int)attributes.size();}
	  ///Retrive the name of the tag.
	  std::string GetName() const {return name;}
	};
	XMLTag OpenTag()
	{
		std::string include = "";
		XMLTag ret;
		XMLAttrib attr;
		bool istag = ExpectOpenTag();
		if( _state == Failure || _state == EndOfFile )
			ret.finish = 0; //there is an error
		else if( !istag )
			ret.finish = 5; //there is no tag opening, probably pure text
		else
		{
			ret.name = ReadOpenTag();
			if( ret.name == "![CDATA[" )
			{
				if( verbose > 1 ) Report("info: opened data block");
				ret.finish = 4; //report that there is a data block
			}
			else
			{
				if( !isTagFinish() ) //have not encountered '</' of the root tag
				{
					if( verbose > 1 ) Report("info: reading tag attributes");
					attr.name = AttributeName();
					while(!isTagEnded() && !isFailure() && !isEof())
					{
						attr.value = AttributeValue();
						if( attr.name == "Include" ) //some file was included
							include = attr.value;
						else ret.attributes.push_back(attr);
						attr.name = AttributeName();
					}
					if( verbose > 1 ) Report("info: finalized tag reading");
					ret.finish = ReadCloseTag(); //retrive '>'
					if( !include.empty() )
					{
						std::string folder = GetFolder(GetStream().src);
						if (!folder.empty()) folder += "/";
						if( verbose > 1 ) Report("info: switching to stream %s",(folder + include).c_str());
						PushStream((folder + include).c_str()); //switch to the included file
					}
				}
				else //encountered '</' of the root tag, no tag was red
				{
					if( verbose > 1 ) Report("info: encountered finalization of tag '</'");
					ret.name = "";
					ret.finish = 3;
				}
			}
		}
		if( verbose > 1 ) Report("info: return tag %s flag %d",ret.name.c_str(),ret.finish);
		return ret;
	}
	bool CloseTag(XMLTag & tag)
	{
		if( tag.finish == 3 ) 
		{
			Report("%s:%d Trying to close the tag that is beyond the last tag",__FILE__,__LINE__);
			return false;
		}
		else if( tag.finish == 2 ) return true; //no need to read anything
		else if( tag.finish == 0 ) return false; //there was a Failure
		else
		{
			if( verbose > 1 ) Report("info: going to finish tag %s",tag.name.c_str());
			return ReadFinishTag(tag.name);
		}
	}
	/// Structure defining entire XML file.
	struct XMLTree
	{
		XMLTag tag; //< tag information, such as name and attributes.
		std::vector<XMLTree> children; //< Children inside XML tag.
		std::string contents; //< Text inside of the tag.
		///Return next occurance of XML tag with the specified
		///name. Returns NumChildren() if not found.
		int FindChild(std::string name, int offset = -1) const
		{
			for(int k = offset+1; k < NumChildren(); ++k)
			if( GetChild(k).GetName() == name )
				return k;
			return NumChildren();
		}
		///Return next occurance of XML attribute with the specified
		///name. Returns NumAttrib() if not found.
		int FindAttrib(std::string name, int offset = -1) const
		{
			for(int k = offset+1; k < NumAttrib(); ++k)
			if( GetAttrib(k).name == name )
				return k;
			return NumAttrib();
		}
		///Retrive a child of current XML tag with number n.
		const XMLTree & GetChild(int n) const {return children[n];}
		///Retrive a child of current XML tag with name
		///Returns NULL if not found.
		const XMLTree * GetChild(std::string name) const
		{
			int n = FindChild(name);
			if (n != NumChildren())
				return &children[n];
			else
				return NULL;
		}
		///Retrive a child of current XML tag with attribute
		///Returns NULL if not found.
		const XMLTree * GetChildWithAttrib(std::string name, std::string value) const
		{
			for (int k = 0; k < NumChildren(); ++k)
			{
				const XMLTree & t = GetChild(k);
				int q = t.FindAttrib(name);
				if (q != t.NumAttrib() && t.GetAttrib(q).value == value)
					return &t;
			}
			return NULL;
		}
		///Retrive number of children.
		int NumChildren() const {return (int)children.size();}
		///Retrive attribute of current XML tag with number n.
		const XMLAttrib & GetAttrib(int n) const {return tag.GetAttrib(n);}
		///Retrive attribute of current XML tag with name.
		///Returns NULL if not found.
		const std::string & GetAttrib(std::string name) const
		{
			int n = FindAttrib(name);
			if(n == NumAttrib()) throw "Attribute not found";
			return GetAttrib(n).value;
		}
		///Retrive number of attributes.
		int NumAttrib() const {return tag.NumAttrib();}
		///Retrive the name of the tag.
		std::string GetName() const {return tag.GetName();}
		///Retrive contents of the tag.
		const std::string & GetContents() const {return contents;}
		
		static std::ostream & Tabs(std::ostream & output, int num)
		{
			while(num > 0) {output.put('\t'); --num;}
			return output;
		}
		void WriteXML(std::ostream & output, int offset = 0) const
		{
			Tabs(output,offset) << "<" << GetName();
			for(int k = 0; k < NumAttrib(); ++k)
			{
				const XMLReader::XMLAttrib & attr = GetAttrib(k);
				output << std::endl;;
				Tabs(output,offset+1) << attr.name << "=\"" << attr.value << "\"";
			}
			if( tag.Stub() ) output << "/";
			output << ">" << std::endl;
			if( tag.Stub() ) return;
			for(int k = 0; k < NumChildren(); ++k)
				GetChild(k).WriteXML(output,offset+1);
			if( !GetContents().empty() )
				Tabs(output,offset+1) << "<![CDATA[" << GetContents() << "]]>" << std::endl;
			Tabs(output,offset) << "</" << GetName() << ">" << std::endl;
		}
	};
	void SkipTag(std::string tagname)
	{
		char c,cb;
		std::string ret;
		std::string inc = "<" + tagname;
		std::string stop = "</"+tagname+">";
		int cnt = 1;
		//ret.size() < stop.size() || ret != stop
		if( verbose )
		{
			Report("info: skip tag %s, stop at %s increase at %s",tagname.c_str(),stop.c_str(),inc.c_str());
			WAITNL;
		}
		do
		{
			c = GetChar();
			if( _state == Failure )
			{
				Report("Unexpected failure while searching for end of tag %s, string %s",tagname.c_str());
				break;
			}
			else if( _state == Failure )
			{
				Report("Unexpected end of file while searching for end of tag %s",tagname.c_str());
				break;
			}
			ret.push_back(c);
			if( ret.size() > stop.size() ) ret.erase(ret.begin());
			if( ret == stop ) 
			{
				cnt--;
				if( verbose )
				{
					Report("info: encountered stop symbols, string %s, cnt %d",ret.c_str(),cnt);
					WAITNL;
				}
			}
			if( ret.size() >= inc.size() && ret.substr(ret.size()-inc.size()) == inc ) //same tag name
			{
				if( verbose )
				{
					Report("info: encountered increase symbols, string %s, cnt %d",ret.c_str(),cnt);
					WAITNL;
				}
				//read until closing scope ends
				do
				{
					cb = c;
					c = GetChar();
				} while( c != '>' );
				//tag was not closed
				if( cb != '/' ) 
				{
					cnt++;
					if( verbose )
					{
						Report("info: tag is not stub, cnt %d",cnt);
						WAITNL;
					}
				}
				else if( verbose )
				{
					Report("info: tag is stub",cnt);
					WAITNL;
				}
			}
		}
		while( cnt != 0 );
		_state = Intro; //wait for next tag
	}
	std::string GetWord(bool stop = '\0')
	{
		std::string ret;
		char c = GetChar();
		while(isspace(c)) c = GetChar();
		while(!isspace(c))
		{
			ret.push_back(c);
			c = GetChar();
			if( c == stop )
			{
				RetChar();
				break;
			}
		}
		return ret;
	}
private:
	std::string ReadUntil(std::string stop)
	{
		char c;
		std::string ret;
		do
		{
			c = GetChar();
			if( _state == Failure )
			{
				Report("Unexpected failure while searching for %s",stop.c_str());
				break;
			}
			else if( _state == Failure )
			{
				Report("Unexpected end of file while searching for %s",stop.c_str());
				break;
			}
			ret.push_back(c);
		}
		while( ret.size() < stop.size() || ret.substr(ret.size()-stop.size()) != stop );

		return ret;
	}
	int ReadXMLSub(XMLTree & root)
	{
		if( verbose )
		{
			Report("info: processing children of %s",root.GetName().c_str());
			WAITNL;
		}
		for(XMLTag t = OpenTag(); !t.Finalize(); t = OpenTag())
		{
			if( verbose )
			{
				Report("info: processing child %s of tag %s, flag %d",t.GetName().c_str(),root.GetName().c_str(),t.finish);
				WAITNL;
			}
			if( t.Finalize() )
			{
				if( verbose )
				{
					Report("info: finalize tag");
					WAITNL;
				}
				break;
			}
			else if( t.RawData() )
			{
				std::string data = ReadUntil("<"); //read until next tag opening
				data.resize(data.size()-1); //drop '<'
				//remove trailing spaces
				while( !data.empty() && isspace(data[data.size()-1]) ) data.resize(data.size()-1);
				if( verbose )
				{
					Report("info: encountered raw data, %s",data.c_str());
					WAITNL;
				}
				root.contents += data;
				RetChar(); //return '<' back into stream
				_state = Intro; //change state to read next tag
			}
			else if( t.BlockData() )
			{
				std::string data = ReadUntil("]]>");
				data.resize(data.size()-3); //drop ]]>
				if( verbose )
				{
					Report("info: encountered block data, %s",data.c_str());
					WAITNL;
				}
				root.contents += data; //read until end of block data
				_state = Intro; //change state to read next tag
			}
			else if( t.Failure() ) 
			{
				Report("Failed opening XML tag");
				WAITNL;
				return -1;
			}
			else
			{
				if( verbose )
				{
					Report("info: attached new child %s to %s",t.GetName().c_str(),root.GetName().c_str());
					WAITNL;
				}
				root.children.push_back(XMLTree());
				root.children.back().tag = t;
				if( root.children.back().tag.Process() )
				{
					if( verbose > 1 ) Report("info: calling ReadXMLSub for tree node");
					if( ReadXMLSub(root.children.back()) == -1 )
						return -1;
				}
				else if( verbose > 1 ) Report("info: no ReadXMLSub, child is tree leaf");

				if( verbose > 1 ) Report("info: closing xml tag %s",root.children.back().tag.GetName().c_str());

				if( !CloseTag(root.children.back().tag) )
				{
					Report("Failed closing XML tag");
					WAITNL;
					return -1;
				}
			}
		}
		if( verbose )
		{
			Report("info: end processing children of %s",root.GetName().c_str());
			WAITNL;
		}
		return 0;
	}
	
public:
	/// Read entire XML file into structure,
	/// it may be more efficient to read the file incrementially, depending on the size.
	/// See mesh_xml_file.cpp for incremential read.
	XMLTree ReadXML()
	{
		XMLTree root;
		root.tag = OpenTag();
		if( verbose ) Report("info: root tag name %s",root.GetName().c_str());
		if( ReadXMLSub(root) == -1 )
			Report("Failed reading XML file");
		CloseTag(root.tag);
		return root;
	}
	
};

#endif //_XML_H
