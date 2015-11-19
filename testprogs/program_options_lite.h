/* ***** BEGIN LICENSE BLOCK *****
 *
 * The MIT License
 *
 * Copyright (c) 2010-2011 BBC Research
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <map>

namespace df {
namespace program_options_lite {

struct Options;

void doHelp(std::ostream& out, Options& opts, unsigned columns = 80);
unsigned parseGNU(Options& opts, unsigned argc, const char* argv[]);
unsigned parseSHORT(Options& opts, unsigned argc, const char* argv[]);
std::list<const char*> scanArgv(Options& opts, unsigned argc, const char* argv[]);
void scanLine(Options& opts, std::string& line);
void scanFile(Options& opts, std::istream& in);
void setDefaults(Options& opts);
void parseConfigFile(Options& opts, const std::string& filename);
bool storePair(Options& opts, const std::string& name, const std::string& value);

/** OptionBase: Virtual base class for storing information relating to a
 * specific option This base class describes common elements.  Type specific
 * information should be stored in a derived class. */
struct OptionBase
{
	OptionBase(const std::string& name, const std::string& desc)
	: opt_string(name), opt_desc(desc)
	{};

	virtual ~OptionBase() {}

	/* parse argument arg, to obtain a value for the option */
	virtual void parse(const std::string& arg) = 0;
	/* set the argument to the default value */
	virtual void setDefault() = 0;

	std::string opt_string;
	std::string opt_desc;
};

/** Type specific option storage */
template<typename T>
struct Option : public OptionBase
{
	Option(const std::string& name, T& storage, const std::string& desc)
	: OptionBase(name, desc), opt_storage(storage)
	{}

	void parse(const std::string& arg);

	void setDefault() {}

	T& opt_storage;
};

/** Type specific option storage (with default value) */
template<typename T>
struct OptionDefault : public Option<T>
{
	OptionDefault(const std::string& name, T& storage, T default_val, const std::string& desc)
	: Option<T>(name, storage, desc), opt_default_val(default_val)
	{}

	void setDefault()
	{
		Option<T>::opt_storage = opt_default_val;
	}

	T opt_default_val;
};

/* Generic parsing */
template<typename T>
inline void
Option<T>::parse(const std::string& arg)
{
	std::istringstream arg_ss (arg,std::istringstream::in);
	if (arg.size() > 2 && arg[0] == '0' && arg[1] == 'x')
		arg_ss >> std::hex;
	arg_ss >> opt_storage;
}

/* string parsing is specialized -- copy the whole string, not just the
 * first word */
template<>
inline void
Option<std::string>::parse(const std::string& arg)
{
	opt_storage = arg;
}

/** Option class for argument handling using a user provided function */
struct OptionFunc : public OptionBase
{
	typedef void (Func)(Options&, const std::string&);

	OptionFunc(const std::string& name, Options& parent_, Func *func_, const std::string& desc)
	: OptionBase(name, desc), parent(parent_), func(func_)
	{}

	void parse(const std::string& arg)
	{
		func(parent, arg);
	}

	void setDefault()
	{
		return;
	}

private:
	Options& parent;
	void (*func)(Options&, const std::string&);
};

class OptionSpecific;
struct Options
{
	~Options();

	OptionSpecific addOptions();

	struct Names
	{
		Names() : opt(0) {};
		~Names()
		{
			if (opt)
				delete opt;
		}
		std::list<std::string> opt_long;
		std::list<std::string> opt_short;
		OptionBase* opt;
	};

	void addOption(OptionBase *opt);

	typedef std::list<Names*> NamesPtrList;
	NamesPtrList opt_list;

	typedef std::map<std::string, NamesPtrList> NamesMap;
	NamesMap opt_long_map;
	NamesMap opt_short_map;
};

/* Class with templated overloaded operator(), for use by Options::addOptions() */
class OptionSpecific
{
public:
	OptionSpecific(Options& parent_) : parent(parent_) {}

	/**
	 * Add option described by name to the parent Options list,
	 *   with storage for the option's value
	 *   with desc as an optional help description
	 */
	template<typename T>
	OptionSpecific&
	operator()(const std::string& name, T& storage, const std::string& desc = "")
	{
		parent.addOption(new Option<T>(name, storage, desc));
		return *this;
	}

	/**
	 * Add option described by name to the parent Options list,
	 *   with storage for the option's value
	 *   with default_val as the default value
	 *   with desc as an optional help description
	 */
	template<typename T>
	OptionSpecific&
	operator()(const std::string& name, T& storage, T default_val, const std::string& desc = "")
	{
		parent.addOption(new OptionDefault<T>(name, storage, default_val, desc));
		return *this;
	}

	/**
	 * Add option described by name to the parent Options list,
	 *   with desc as an optional help description
	 * instead of storing the value somewhere, a function of type
	 * OptionFunc::Func is called.  It is upto this function to correctly
	 * handle evaluating the option's value.
	 */
	OptionSpecific&
	operator()(const std::string& name, OptionFunc::Func *func, const std::string& desc = "")
	{
		parent.addOption(new OptionFunc(name, parent, func, desc));
		return *this;
	}
private:
	Options& parent;
};

}; /* namespace: program_options_lite */
}; /* namespace: df */
