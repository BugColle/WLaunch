
// WLaunchView.cpp : CWLaunchView クラスの実装
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS は、プレビュー、縮小版、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "WLaunch.h"
#endif

#include "WLaunchDoc.h"
#include "WLaunchView.h"

#include <shellapi.h>
#include <shlobj.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CGroupNameDialog : public CDialogEx
{
public:
	CString name;

	explicit CGroupNameDialog(const CString& groupName)
		: CDialogEx(IDD_GROUP_RENAME), name(groupName)
	{
	}

protected:
	void DoDataExchange(CDataExchange* pDX) override
	{
		CDialogEx::DoDataExchange(pDX);
		DDX_Text(pDX, IDC_GROUP_NAME, name);
	}
};


// CWLaunchView

IMPLEMENT_DYNCREATE(CWLaunchView, CView)

BEGIN_MESSAGE_MAP(CWLaunchView, CView)
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CWLaunchView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CWLaunchView コンストラクション/デストラクション

CWLaunchView::CWLaunchView() noexcept
{
	LoadGroups();
}

CWLaunchView::~CWLaunchView()
{
}

BOOL CWLaunchView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CView::PreCreateWindow(cs);
}

// CWLaunchView 描画

void CWLaunchView::OnDraw(CDC* pDC)
{
	CWLaunchDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->FillSolidRect(clientRect, RGB(248, 249, 252));

	const int tabY = 20;
	const int tabHeight = 38;
	int tabX = 20;
	m_groupRects.clear();
	for (size_t i = 0; i < m_groups.size(); ++i)
	{
		CSize textSize = pDC->GetTextExtent(m_groups[i].name);
		int width = max(100, textSize.cx + 32);
		CRect tab(tabX, tabY, tabX + width, tabY + tabHeight);
		m_groupRects.push_back(tab);
		COLORREF tabColor = i == static_cast<size_t>(m_selectedGroup) ? RGB(226, 237, 255) : RGB(238, 240, 244);
		pDC->FillSolidRect(&tab, tabColor);
		pDC->SetTextColor(i == static_cast<size_t>(m_selectedGroup) ? RGB(35, 91, 170) : RGB(75, 82, 94));
		pDC->DrawText(m_groups[i].name, tab, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		tabX += width + 6;
	}

	m_addGroupRect.SetRect(tabX + 4, tabY, tabX + 116, tabY + tabHeight);
		pDC->FillSolidRect(&m_addGroupRect, RGB(233, 235, 240));
	pDC->SetTextColor(RGB(69, 76, 89));
	pDC->DrawText(_T("+ グループ"), m_addGroupRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	CRect contentRect(20, tabY + 56, clientRect.right - 20, clientRect.bottom - 20);
	pDC->FillSolidRect(&contentRect, RGB(255, 255, 255));
	m_addItemRect.SetRect(contentRect.right - 142, contentRect.top + 14, contentRect.right - 14, contentRect.top + 48);
	pDC->SetTextColor(RGB(46, 111, 214));
	pDC->DrawText(_T("+ アプリ追加"), m_addItemRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	pDC->SetTextColor(RGB(95, 103, 117));
	CString groupTitle;
	groupTitle.Format(_T("%s  ・  %u 個のアプリ"), m_groups[m_selectedGroup].name.GetString(), static_cast<unsigned int>(m_groups[m_selectedGroup].items.size()));
	pDC->TextOut(contentRect.left + 20, contentRect.top + 23, groupTitle);

	m_itemRects.clear();
	const int cellWidth = 112;
	const int cellHeight = 112;
	const int startX = contentRect.left + 20;
	const int startY = contentRect.top + 70;
	const int columns = max(1, (contentRect.Width() - 36) / cellWidth);
	const std::vector<LauncherItem>& items = m_groups[m_selectedGroup].items;
	for (size_t i = 0; i < items.size(); ++i)
	{
		int column = static_cast<int>(i) % columns;
		int row = static_cast<int>(i) / columns;
		CRect cell(startX + column * cellWidth, startY + row * cellHeight,
			startX + column * cellWidth + cellWidth - 8, startY + row * cellHeight + cellHeight - 8);
		m_itemRects.push_back(cell);
		pDC->FillSolidRect(&cell, RGB(248, 249, 252));

		SHFILEINFO fileInfo = {};
		if (SHGetFileInfo(items[i].path, 0, &fileInfo, sizeof(fileInfo), SHGFI_ICON | SHGFI_LARGEICON))
		{
			int iconX = cell.left + (cell.Width() - 40) / 2;
			DrawIconEx(pDC->GetSafeHdc(), iconX, cell.top + 12, fileInfo.hIcon, 40, 40, 0, nullptr, DI_NORMAL);
			DestroyIcon(fileInfo.hIcon);
		}
		pDC->SetTextColor(RGB(47, 54, 66));
		CRect labelRect(cell.left + 4, cell.top + 60, cell.right - 4, cell.bottom - 4);
		pDC->DrawText(items[i].label, labelRect, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
	}
}


// CWLaunchView の印刷


void CWLaunchView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CWLaunchView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 既定の印刷準備
	return DoPreparePrinting(pInfo);
}

void CWLaunchView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CWLaunchView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}

void CWLaunchView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWLaunchView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	CPoint clientPoint(point);
	ScreenToClient(&clientPoint);
	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	for (size_t i = 0; i < m_groupRects.size(); ++i)
	{
		if (m_groupRects[i].PtInRect(clientPoint))
		{
			menu.AppendMenu(MF_STRING, 1, _T("このグループを削除"));
			menu.AppendMenu(MF_STRING, 3, _T("グループ名を変更"));
			UINT command = ::TrackPopupMenu(menu.GetSafeHmenu(),
				TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, 0, GetSafeHwnd(), nullptr);
			PostMessage(WM_NULL);
			if (command == 1)
				RemoveGroup(static_cast<int>(i));
			else if (command == 3)
				RenameGroup(static_cast<int>(i));
			return;
		}
	}
	for (size_t i = 0; i < m_itemRects.size(); ++i)
	{
		if (m_itemRects[i].PtInRect(clientPoint))
		{
			menu.AppendMenu(MF_STRING, 2, _T("このアプリを削除"));
			UINT command = ::TrackPopupMenu(menu.GetSafeHmenu(),
				TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, 0, GetSafeHwnd(), nullptr);
			PostMessage(WM_NULL);
			if (command == 2)
				RemoveItem(static_cast<int>(i));
			return;
		}
	}
}

void CWLaunchView::OnLButtonDown(UINT nFlags, CPoint point)
{
	for (size_t i = 0; i < m_groupRects.size(); ++i)
	{
		if (m_groupRects[i].PtInRect(point))
		{
			m_selectedGroup = static_cast<int>(i);
			Invalidate();
			return;
		}
	}
	if (m_addGroupRect.PtInRect(point))
	{
		AddGroup();
		return;
	}
	if (m_addItemRect.PtInRect(point))
	{
		AddItems();
		return;
	}
	for (size_t i = 0; i < m_itemRects.size(); ++i)
	{
		if (m_itemRects[i].PtInRect(point))
		{
			LaunchItem(static_cast<int>(i));
			return;
		}
	}
	CView::OnLButtonDown(nFlags, point);
}

void CWLaunchView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	Invalidate();
}

void CWLaunchView::LoadGroups()
{
	CWinApp* app = AfxGetApp();
	int groupCount = min(100, max(0, app->GetProfileInt(_T("Launcher"), _T("GroupCount"), 0)));
	for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex)
	{
		CString section;
		section.Format(_T("Launcher\\Group%d"), groupIndex);
		LauncherGroup group;
		group.name = app->GetProfileString(section, _T("Name"), _T("グループ"));
		int itemCount = min(200, max(0, app->GetProfileInt(section, _T("ItemCount"), 0)));
		for (int itemIndex = 0; itemIndex < itemCount; ++itemIndex)
		{
			CString key;
			key.Format(_T("Item%d"), itemIndex);
			CString path = app->GetProfileString(section, key);
			if (path.IsEmpty())
				continue;
			LauncherItem item;
			item.path = path;
			int slash = max(path.ReverseFind(_T('\\')), path.ReverseFind(_T('/')));
			item.label = path.Mid(slash + 1);
			group.items.push_back(item);
		}
		m_groups.push_back(group);
	}
	if (m_groups.empty())
	{
		LauncherGroup group;
		group.name = _T("グループ 1");
		m_groups.push_back(group);
	}
	m_selectedGroup = 0;
}

void CWLaunchView::SaveGroups() const
{
	CWinApp* app = AfxGetApp();
	app->WriteProfileInt(_T("Launcher"), _T("GroupCount"), static_cast<int>(m_groups.size()));
	for (size_t groupIndex = 0; groupIndex < m_groups.size(); ++groupIndex)
	{
		CString section;
		section.Format(_T("Launcher\\Group%u"), static_cast<unsigned int>(groupIndex));
		app->WriteProfileString(section, _T("Name"), m_groups[groupIndex].name);
		app->WriteProfileInt(section, _T("ItemCount"), static_cast<int>(m_groups[groupIndex].items.size()));
		for (size_t itemIndex = 0; itemIndex < m_groups[groupIndex].items.size(); ++itemIndex)
		{
			CString key;
			key.Format(_T("Item%u"), static_cast<unsigned int>(itemIndex));
			app->WriteProfileString(section, key, m_groups[groupIndex].items[itemIndex].path);
		}
	}
}

void CWLaunchView::AddGroup()
{
	LauncherGroup group;
	group.name.Format(_T("グループ %u"), static_cast<unsigned int>(m_groups.size() + 1));
	m_groups.push_back(group);
	m_selectedGroup = static_cast<int>(m_groups.size()) - 1;
	SaveGroups();
	Invalidate();
}

void CWLaunchView::AddItems()
{
	const TCHAR filter[] = _T("アプリとショートカット (*.exe;*.lnk;*.bat;*.cmd;*.url;*.msc;*.cpl)|*.exe;*.lnk;*.bat;*.cmd;*.url;*.cpl|すべてのファイル (*.*)|*.*||");
	CFileDialog dialog(TRUE, _T("exe"), nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		filter, this);
	TCHAR documentsPath[MAX_PATH] = {};
	if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, documentsPath)))
		dialog.m_ofn.lpstrInitialDir = documentsPath;
	dialog.m_ofn.nFilterIndex = 2;
	if (dialog.DoModal() != IDOK)
		return;

	CString path = dialog.GetPathName();
	LauncherItem item;
	item.path = path;
	int slash = max(path.ReverseFind(_T('\\')), path.ReverseFind(_T('/')));
	item.label = path.Mid(slash + 1);
	m_groups[m_selectedGroup].items.push_back(item);
	SaveGroups();
	Invalidate();
}

void CWLaunchView::RemoveItem(int index)
{
	if (index < 0 || index >= static_cast<int>(m_groups[m_selectedGroup].items.size()))
		return;
	m_groups[m_selectedGroup].items.erase(m_groups[m_selectedGroup].items.begin() + index);
	SaveGroups();
	Invalidate();
}

void CWLaunchView::RemoveGroup(int index)
{
	if (index < 0 || index >= static_cast<int>(m_groups.size()))
		return;
	m_groups.erase(m_groups.begin() + index);
	if (m_groups.empty())
	{
		LauncherGroup group;
		group.name = _T("グループ 1");
		m_groups.push_back(group);
	}
	m_selectedGroup = min(m_selectedGroup, static_cast<int>(m_groups.size()) - 1);
	SaveGroups();
	Invalidate();
}

void CWLaunchView::RenameGroup(int index)
{
	if (index < 0 || index >= static_cast<int>(m_groups.size()))
		return;

	CGroupNameDialog dialog(m_groups[index].name);
	if (dialog.DoModal() != IDOK)
		return;

	dialog.name.Trim();
	if (dialog.name.IsEmpty())
	{
		AfxMessageBox(_T("グループ名を入力してください。"), MB_ICONINFORMATION);
		return;
	}

	m_groups[index].name = dialog.name;
	SaveGroups();
	Invalidate();
}

void CWLaunchView::LaunchItem(int index)
{
	if (index < 0 || index >= static_cast<int>(m_groups[m_selectedGroup].items.size()))
		return;
	const LauncherItem& item = m_groups[m_selectedGroup].items[index];
	if (reinterpret_cast<INT_PTR>(ShellExecute(GetSafeHwnd(), _T("open"), item.path, nullptr, nullptr, SW_SHOWNORMAL)) <= 32)
		AfxMessageBox(_T("アプリケーションを起動できませんでした。登録先を確認してください。"), MB_ICONWARNING);
}


// CWLaunchView の診断

#ifdef _DEBUG
void CWLaunchView::AssertValid() const
{
	CView::AssertValid();
}

void CWLaunchView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWLaunchDoc* CWLaunchView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWLaunchDoc)));
	return (CWLaunchDoc*)m_pDocument;
}
#endif //_DEBUG


// CWLaunchView メッセージ ハンドラー
