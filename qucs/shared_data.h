#include "data.h"
/* -------------------------------------------------------------------------------- */
// rename to SharedData*?
class CommonDataIndex : public CommonData{
public:
	explicit CommonDataIndex() : CommonData() {
	}
	~CommonDataIndex();

private:
	CommonDataIndex(CommonDataIndex const& p);

public:
	CommonData* clone();
	void push_back(Data* d);

private:
	std::multimap<std::string, CommonData const*> _m;
};
/* -------------------------------------------------------------------------------- */
void CommonDataIndex::push_back(Data* d)
{

}
