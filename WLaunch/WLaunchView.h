
// WLaunchView.h : CWLaunchView クラスのインターフェイス
//

#pragma once

#include <vector>

struct LauncherItem
{
	CString path;
	CString label;
};

struct LauncherGroup
{
	CString name;
	std::vector<LauncherItem> items;
};

class CWLaunchView : public CView
{
protected: // シリアル化からのみ作成します。
	CWLaunchView() noexcept;
	DECLARE_DYNCREATE(CWLaunchView)

// 属性
public:
	CWLaunchDoc* GetDocument() const;

// 操作
public:

// オーバーライド
public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画するためにオーバーライドされます。
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 実装
public:
	virtual ~CWLaunchView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	std::vector<LauncherGroup> m_groups;
	std::vector<CRect> m_groupRects;
	std::vector<CRect> m_itemRects;
	CRect m_addGroupRect;
	CRect m_addItemRect;
	int m_selectedGroup = 0;

	void LoadGroups();
	void SaveGroups() const;
	void AddGroup();
	void AddItems();
	void RemoveItem(int index);
	void RemoveGroup(int index);
	void RenameGroup(int index);
	void LaunchItem(int index);

// 生成された、メッセージ割り当て関数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // WLaunchView.cpp のデバッグ バージョン
inline CWLaunchDoc* CWLaunchView::GetDocument() const
   { return reinterpret_cast<CWLaunchDoc*>(m_pDocument); }
#endif

