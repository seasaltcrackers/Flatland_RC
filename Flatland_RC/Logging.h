#pragma once
#include <iostream>
#include <string>
#include <format>

class Logging
{
public:

	template<typename... Args>
	static void Log(const std::string_view& log, Args&&... args)
	{
		std::cout << "LOG: " << std::vformat(log, std::make_format_args(args...)) << std::endl;
	}

	template<typename... Args>
	static void LogError(const std::string_view& error, Args&&... args)
	{
		std::cout << "ERROR: " << std::vformat(error, std::make_format_args(args...)) << std::endl;
	}

	template<typename... Args>
	static void ThrowError(const std::string_view& error, Args&&... args)
	{
		std::cout << "CRITICAL ERROR: " << std::vformat(error, std::make_format_args(args...)) << std::endl;
		throw std::runtime_error(std::vformat(error, std::make_format_args(args...)));
	}
};

