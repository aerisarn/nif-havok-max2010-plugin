#ifndef __NIFGUI_H__
#define __NIFGUI_H__

/* Combo box control wrapper. */
class NpComboBox
{

public:

	NpComboBox(HWND hWnd=NULL);

	void			init(HWND hWnd);

	int				add(const TCHAR *);

	int				count() const;
	void			select(int i);
	int 			selection() const;

	HWND			mWnd;
};

#endif __NIFGUI_H__
