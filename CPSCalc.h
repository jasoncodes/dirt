#ifndef CPSCalc_H_
#define CPSCalc_H_

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(off_t, HistoryArray);

class CPSCalc
{

public:
	CPSCalc();
	virtual ~CPSCalc();

	void Reset(off_t resume_offset = 0);

	// this func must be called every 1000 ms
	off_t Update(off_t pos);

	off_t GetCPS() { return m_cps; }
	bool IsCPSValid() { return m_history_filled > 1; }

protected:
	HistoryArray m_history;
	int m_history_filled;
	off_t m_resume_offset;
	off_t m_cps;
	off_t m_pos;

};

#endif
