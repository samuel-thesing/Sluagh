#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "sluagh/export.h"
#include "sluagh/utils/Logger.h"

class SLUAGH_EXPORT Arguments {
private:
	std::string tool_name;

	std::unordered_set<std::string> flags;
	std::unordered_map<std::string, std::string> named_args;
	std::vector<std::string> args;

	int version_major, version_minor, version_patch;

	struct Option {
		std::string name;
		std::string description;
		std::string abbreviation;
	};

	std::string tool_description;
	std::vector<Option> options;

public:
	Arguments(std::string tool_name);

	void addOptionFlag(std::string name, std::string description,
		std::string abbreviation = "");
	void addOptionNamedValue(std::string name, std::string value_name,
		std::string description, std::string abbreviation = "");
	void setGeneralDescription(std::string general_description);

	bool parseArgs(int argc, char **argv);

	void printVersion();
	void printHelp();

	unsigned getArgCount() const {
		return static_cast<unsigned>(args.size());
	}
	unsigned isArgsEmpty() const { return args.empty(); }

	bool hasNamedArg(const std::string &name);
	std::optional<std::string> getNamedArg(const std::string &name);
	bool hasFlag(const std::string &name);

	Arguments &operator>>(char &c);
	Arguments &operator>>(std::string &s);
	Arguments &operator>>(int &i);
	Arguments &operator>>(long long &l);
	Arguments &operator>>(float &f);
	Arguments &operator>>(double &d);

	void removeFlag(const std::string &name);
	void removeNamedArg(const std::string &name);
	void clearFlags();
	void clearNamedArgs();
	void clearArgs();
	void clear();
};

#endif // ARGUMENTS_H
