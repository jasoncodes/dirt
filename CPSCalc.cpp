#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "CPSCalc.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(HistoryArray);

const int num_history = 5;

const int last_history = num_history - 1;

CPSCalc::CPSCalc()
{
	m_history.Insert(0, 0, num_history);
	Reset();
}

CPSCalc::~CPSCalc()
{
}

void CPSCalc::Reset()
{
	m_cps = -1;
	m_history_filled = 0;
	m_pos = 0;
}

off_t CPSCalc::Update(off_t pos)
{

	m_pos = pos;

    if (pos > 0)
	{

		for (int i = 1; i < num_history; ++i)
		{
			m_history[i - 1] = m_history[i];
		}

		m_history[last_history] = pos;

		if (m_history_filled > 0)
		{
			int first_history = last_history - m_history_filled;
			m_cps =
				(m_history[last_history] - m_history[first_history])
				/ m_history_filled;
		}
		else
		{
			m_cps = -1;
		}

		m_history_filled = wxMin(m_history_filled + 1, last_history);

	}
	else
	{
		m_cps = -1;
	}

	return m_cps;

}
