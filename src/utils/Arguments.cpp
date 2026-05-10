#include "sluagh/utils/Arguments.h"

#include "sluagh/utils/macros.h"

#include <algorithm>
#include <iostream>
#include <utility>

Arguments::Arguments(std::string tool_name)
	: tool_name(std::move(tool_name)), version_major(SLUAGH_VERSION_MAJOR),
	  version_minor(SLUAGH_VERSION_MINOR), version_patch(SLUAGH_VERSION_PATCH) {}

void Arguments::addOptionFlag(
	std::string name, std::string description, std::string abbreviation) {
	if (abbreviation.empty()) {
		description = std::format("\t--{}\n\t\t{}\n", name, description);
	} else {
		description = std::format(
			"\t-{}, --{}\n\t\t{}\n", abbreviation, name, description);
	}
	options.emplace_back(name, description, abbreviation);
}

void Arguments::addOptionNamedValue(std::string name, std::string value_name,
	std::string description, std::string abbreviation) {
	if (abbreviation.empty()) {
		description =
			std::format("\t--{}={}\n\t\t{}\n", name, value_name, description);
	} else {
		description = std::format("\t-{}={}, --{}={}\n\t\t{}\n", abbreviation,
			value_name, name, value_name, description);
	}
	options.emplace_back(name, description, abbreviation);
}

void Arguments::setGeneralDescription(std::string general_description) {
	this->tool_description = std::move(general_description);
}

bool Arguments::parseArgs(int argc, char **argv) {
	int i = 1;
	bool error_occured = false;
	while (i < argc) {
		std::string s = argv[i];
		size_t value_idx = s.find_first_of('=');
		std::string value =
			value_idx != std::string::npos ? s.substr(value_idx + 1) : "";
		s = s.substr(0, value_idx);
		std::string name;
		if (s.starts_with("--")) {
			name = s.substr(2);
		} else if (s.starts_with("-")) {
			auto abbr = s.substr(1);
			auto option = std::ranges::find_if(options,
				[&](const Option &opt) { return opt.abbreviation == abbr; });
			if (option == options.end()) {
				Logger::error("Unknown option '{}'", abbr);
				error_occured = true;
				i++;
				continue;
			}
			name = option->name;
		} else {
			break;
		}

		if (value_idx == std::string::npos) {
			if (!flags.emplace(name).second) {
				Logger::warn("Multiple definitions of flag '{}'", name);
			}
		} else {
			if (!named_args.emplace(name, value).second) {
				Logger::warn("Multiple definitions of option '{}'", name);
			}
		}

		++i;
	}

	while (i < argc) {
		std::string s = argv[i];
		args.emplace_back(s);
		i++;
	}

	return !error_occured;
}

void Arguments::printVersion() {
	std::cout << tool_name << " " << version_major << "." << version_minor
			  << "." << version_patch << std::endl;
}

void Arguments::printHelp() {
	std::ranges::sort(options,
		[](const Option &a, const Option &b) { return a.name < b.name; });
	std::cout << "SYNOPSIS\n\t" << tool_name
			  << " [--option=value | --flag]...\n"
			  << std::endl
			  << "DESCRIPTION\n"
			  << tool_description << std::endl;
	for (const auto &opt : options) {
		std::cout << opt.description << std::endl;
	}
}

bool Arguments::hasNamedArg(const std::string &name) {
	return named_args.contains(name);
}

std::optional<std::string> Arguments::getNamedArg(const std::string &name) {
	const auto it = named_args.find(name);
	return it != named_args.end() ? std::optional{it->second} : std::nullopt;
}

bool Arguments::hasFlag(const std::string &name) {
	return flags.contains(name);
}

Arguments &Arguments::operator>>(char &c) {
	auto s = args.front();
	args.erase(args.begin());

	if (s.size() != 1) {
		Logger::error(
			"Expected exactly 1 character. Got {} instead.", s.size());
		return *this;
	}

	c = s.front();
	return *this;
}

Arguments &Arguments::operator>>(std::string &s) {
	std::string str = args.front();
	args.erase(args.begin());
	s = str;
	return *this;
}

Arguments &Arguments::operator>>(int &i) {
	try {
		i = std::stoi(args.front());
	} catch (const std::invalid_argument &e) {
		UNUSED(e)
		Logger::error("Failed to parse integer from '{}'", args.front());
	} catch (std::out_of_range &e) {
		UNUSED(e)
		Logger::error(
			"Failed to parse integer from '{}'. The number was out of range.",
			args.front());
	}
	args.erase(args.begin());
	return *this;
}

Arguments &Arguments::operator>>(long long &l) {
	try {
		l = std::stoll(args.front());
	} catch (const std::invalid_argument &e) {
		UNUSED(e)
		Logger::error("Failed to parse long long from '{}'", args.front());
	} catch (std::out_of_range &e) {
		UNUSED(e)
		Logger::error(
			"Failed to parse long long from '{}'. The number was out of range.",
			args.front());
	}
	args.erase(args.begin());
	return *this;
}

Arguments &Arguments::operator>>(float &f) {
	try {
		f = std::stof(args.front());
	} catch (const std::invalid_argument &e) {
		UNUSED(e)
		Logger::error("Failed to parse float from '{}'", args.front());
	} catch (std::out_of_range &e) {
		UNUSED(e)
		Logger::error(
			"Failed to parse float from '{}'. The number was out of range.",
			args.front());
	}
	args.erase(args.begin());
	return *this;
}

Arguments &Arguments::operator>>(double &d) {
	try {
		d = std::stod(args.front());
	} catch (const std::invalid_argument &e) {
		UNUSED(e)
		Logger::error("Failed to parse double from '{}'", args.front());
	} catch (std::out_of_range &e) {
		UNUSED(e)
		Logger::error(
			"Failed to parse double from '{}'. The number was out of range.",
			args.front());
	}
	args.erase(args.begin());
	return *this;
}

void Arguments::removeFlag(const std::string &name) { flags.erase(name); }

void Arguments::removeNamedArg(const std::string &name) {
	named_args.erase(name);
}

void Arguments::clearFlags() { flags.clear(); }

void Arguments::clearNamedArgs() { named_args.clear(); }

void Arguments::clearArgs() { args.clear(); }

void Arguments::clear() {
	clearFlags();
	clearNamedArgs();
	clearArgs();
}
