#ifndef TINYWEBSERVER_TOOLS_NOCOPYABLE
#define TINYWEBSERVER_TOOLS_NOCOPYABLE

class nocopyable{
public:
	nocopyable(const nocopyable&) = delete;
	nocopyable& operator=(const nocopyable&) = delete;
protected:
	nocopyable() = default;
	~nocopyable() = default;
};
#endif
