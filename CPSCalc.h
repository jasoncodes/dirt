#ifndef CPSCalc_H_
#define CPSCalc_H_

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(wxLongLong_t, HistoryArray);

class CPSCalc
{

public:
	CPSCalc();
	virtual ~CPSCalc();

	void Reset();

	// this func must be called every 1000 ms
	wxLongLong_t Update(wxLongLong_t pos);

	wxLongLong_t GetCPS() { return m_cps; }
	bool IsCPSValid() { return m_history_filled > 1; }

protected:
	HistoryArray m_history;
	int m_history_filled;
	wxLongLong_t m_cps;
	wxLongLong_t m_pos;

};

#endif
