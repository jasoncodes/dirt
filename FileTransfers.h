#ifndef FileTransfers_H_
#define FileTransfers_H_

#include <wx/dynarray.h>

class Client;
class FileTransfer;

WX_DECLARE_OBJARRAY(FileTransfer, FileTransferArray);

class FileTransfers : public wxEvtHandler
{

	friend FileTransfer;

public:
	FileTransfers(Client *client);
	virtual ~FileTransfers();

	int GetTransferCount();

	int FindTransfer(int transferid);

	const FileTransfer& GetTransferById(int transferid);
	const FileTransfer& GetTransferByIndex(int index);

	void Test();

	void DeleteTransfer(int transferid);

protected:
	void OnTimer(wxTimerEvent &event);

protected:
	int GetNewId();

protected:
	Client *m_client;
	FileTransferArray m_transfers;
	wxTimer *tmr;

private:
	DECLARE_EVENT_TABLE()

};

#endif
