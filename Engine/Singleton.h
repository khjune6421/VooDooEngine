#pragma once

template <typename T>
class Singleton
{
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

protected:
	Singleton() = default;
	~Singleton() = default;

public:
	static T& instance() { static T _instance; return _instance; }
};