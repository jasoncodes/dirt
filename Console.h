#ifndef Console_H_
#define Console_H_

class ConsoleReadThread;

class Console : public wxEvtHandler
{

public:

	Console(bool no_input = false);
	virtual ~Console();

	virtual void ExitMainLoop();
	virtual void Output(const wxString &line);

protected:
	void OnTextEnter(wxCommandEvent& event);

protected:
	virtual void OnInput(const wxString &line) = 0;
	virtual void OnEOF() = 0;

protected:
	ConsoleReadThread *m_read_thread;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(Console)

};

#endif
