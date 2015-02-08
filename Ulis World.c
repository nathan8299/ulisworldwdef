/*	같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같	PROJECT:	Ulis World		KIND:		Window Definition function (WDEF)		FILES:		Ulis World.c	// Source code for the WDEF				Ulis World.r	// This file can contain de-Rez-ed versions of any additionally								// needed resources (cicns, ICONs, PICTs ...)				Ulis World.make	// MakeFile for MPW-C.				Ulis World		// WDEF-Tester file containing the compiled version of the WDEF		COPYRIGHT:	(C) Copyright 1996 by Uli Kusterer, all rights reserved.				This WDEF and its source code are FreeWare, but All rights remain with their				author. You may use derivatives of this project or the project as it is here				for any purpose you wish, as long as you give credits to their author some-				where visible to the users.					REACH ME AT:				COMPUSERVE:	101646,3646								PURPOSE:	This project was created to serve as a template for creating WDEFs. It supports				different highlighting states, colorized window frames, Zoom-, Grow- and Close-				boxes, and it displays a window title.		REVISION HISTORY:				1.0		First public release. WDEF doesn't look good and has just rudimentary						graphics.				1.4		Removed CalcVisStrucRegion because it didn't consider other apps'						windows and so the grow box was drawn on top of them when in background.						Now uses the toolbox call ClipAbove instead. Added white dropshadow-						embossed effect to title. Now uses DrawRectParts instead of LineTo. Window						Still has 3-D borders when not highlighted. Added four pixel frame around						contentRegion. Grow box now part of border. For compatibility with some						apps, grow box isn't drawn when frame is drawn (though recommended by						apple). Added dBoxProc, plainDBoxProc & altDBoxProc variations.				1.5		Added Rallye stripes to make highlighted windows more obvious. Corrected						Errors in variation codes. Added movableDBoxProc variation. Now uses						bounds of contents region instead of recalculating the window's rect						every time -> less calculations, more compatibility to WindowShade. Grow						box parts lying inside the content region are clipped according to the						clipRect. Only sets Color Port when really needed. Some comments have						been corrected, too.				1.6		Fixed bug that caused initially invisible windows to be uninitialized.						-> moved check for (*window).visible				1.7		Fixed bug that had big ZoomBox always watching you.				1.8		Title bar height is now calculated using the System Font's height.							같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */#define SystemSevenOrLater	1		// Makes WDEF smaller through removing System 6 support.// --------------------------------------------------------------------------//  Define one of these to compile light or dark highlighted windows:#define FrontDark		true		// FrontDark or FrontLight#define BackDark		true		// BackDark or BackLight// --------------------------------------------------------------------------// INCLUDE STATEMENTS#include <Windows.h>				// For part codes and WStateData#include <Memory.h>					// For NewHandle and DisposeHandle#include <QuickDraw.h>				// For the drawings#include <Fonts.h>					// For the window Title#include <Types.h>					// Without this I'd have no Rects or Points#include <ToolUtils.h>				// Here are HiWord, LoWord and more#include <GestaltEqu.h>				// I need this for my color check#include <QuickdrawText.h>			// This is needed to get info about the System Font// CONSTANTS FOR DRAWING#define kGrowBoxSize		16		// Size of the Grow Box#define kTitleNameBorder	2		// Top & bottom borders for title#define kTitleBarHeight		(MyFontData.ascent +MyFontData.descent +kTitleNameBorder +kTitleNameBorder)									// Height of the title Bar#if FrontDark						// Use light color for active  #define kFrameCol		52428		// Color for the frame's Background  #define kShadeCol		39321		// Color for the frame's lower right parts#else								// Use dark color for active  #define kFrameCol		61166		// Color for the frame's Background  #define kShadeCol		52428		// Color for the frame's lower right parts#endif#define kGadgetCol		43690		// Color for close, zoom & grow boxes#define kTopGrowCol		48059		// Color for upper grow rect#define	kHighlightCol	30583		// Color for rallye stripes.#define kBlackCol		0			// Simple black#define kWhiteCol		65535		// Simple white#define kDimmedCol		32767		// Color for dimmed (Disabled) parts when the window's									// not highlighted#if BackDark						// Use dark color for inactive  #define kDimFrameCol	52428		// Color for background when window is not highlighted  #define kDimShadeCol	39321		// Color for the frame's lower right parts when disabled#else								// Use light color for inactive  #define kDimFrameCol	61166		// Color for background when window is not highlighted  #define kDimShadeCol	52428		// Color for the frame's lower right parts when disabled#endif// DrawRectParts possible choicesenum {	DR_Top			=	1,	DR_Left,	DR_TopLeft,	DR_Bottom,	DR_TopBot,	DR_LeftBot,	DR_TopLeftBot,	DR_Right,	DR_TopRight,	DR_LeftRight,	DR_TopLeftRight,	DR_BotRight,	DR_TopBotRight,	DR_LeftBotRight,	DR_Full};// FORWARD DECLARATIONSvoid DrawMessage ( short varCode, WindowPeek winPtr, Boolean hasColor );void DrawGoAwayMessage ( short varCode, WindowPeek winPtr, Boolean hasColor );void DrawZoomMessage ( short varCode, WindowPeek winPtr, Boolean hasColor );long HitMessage ( short varCode, WindowPeek winPtr, long param );void CalcRgnsMessage ( short varCode, WindowPeek winPtr, long param );void NewMessage ( short varCode, WindowPeek winPtr, long param );void DisposeMessage ( short varCode, WindowPeek winPtr, long param );void GrowRectMessage ( short varCode, WindowPeek winPtr, Rect *aRect );void GrowIconMessage ( short varCode, WindowPeek winPtr, long param, Boolean hasColor );void GlobalWRect ( WindowPeek winPtr, Rect *aRect );void GlobalContentRect ( WindowPeek winPtr, Rect *aRect );void SetColor ( short redColor, short greenColor, short blueColor );Boolean CheckColor ( WindowPeek winPtr );void SetColorPort ( Boolean hasColor );void CopyRect ( Rect *s, Rect *d);void DrawRectParts ( Rect *aRect, short choice );// -----------------------------------------------------------------------------------------------// This is the main procedure. It calls the routines which handle the requested tasks.// -----------------------------------------------------------------------------------------------pascal long main ( short varCode, WindowPtr theWindow, short message, long param ){	WindowPeek		winPtr;	Boolean			hasColor;		winPtr = (WindowPeek) theWindow;		switch (message)	{		case wDraw:			if (!(*winPtr).visible) return (0);			hasColor = CheckColor ( winPtr );			SetColorPort ( hasColor );			switch ( LoWord( param ) )			{				case 0:					DrawMessage (varCode, winPtr, hasColor);					break;									case wInGoAway:					DrawGoAwayMessage (varCode, winPtr, hasColor);					break;								case wInZoomIn:				case wInZoomOut:					DrawZoomMessage (varCode, winPtr, hasColor);					break;			}			break;				case wHit:			if (!(*winPtr).visible) return (0);			return (HitMessage (varCode, winPtr, param));			break;				case wCalcRgns:			CalcRgnsMessage (varCode, winPtr, param);			break;				case wNew:			NewMessage (varCode, winPtr, param);			break;				case wDispose:			DisposeMessage (varCode, winPtr, param);			break;				case wGrow:			if (!(*winPtr).visible) return (0);			GrowRectMessage (varCode, winPtr, (RectPtr) param);			break;				case wDrawGIcon:			if (!(*winPtr).visible) return (0);			hasColor = CheckColor ( winPtr );			SetColorPort ( hasColor );			GrowIconMessage (varCode, winPtr, param, hasColor);			break;			}	return (0);}// -----------------------------------------------------------------------------------------------// This is the procedure that calculates the structure and content regions of the different// window types.// -----------------------------------------------------------------------------------------------void CalcRgnsMessage ( short varCode, WindowPeek winPtr, long /*param*/ ){	Rect		aRect;	RgnHandle	myRgn;	FontInfo	MyFontData;		GlobalWRect ( winPtr, &aRect );		GetFontInfo ( &MyFontData );		RectRgn ( winPtr->contRgn, &aRect );		if ( varCode != 2 && varCode != 3 )		InsetRect ( &aRect, -6, -6);	else		InsetRect ( &aRect, -1, -1);		if ( varCode != 1 && varCode != 2 && varCode != 3 )	{		aRect.top += 5;		aRect.top -= kTitleBarHeight;	}		RectRgn ( winPtr->strucRgn, &aRect );		if ( varCode == 3 )	{		myRgn = NewRgn ();		OffsetRect ( &aRect, 2, 2 );		RectRgn ( myRgn, &aRect );		UnionRgn ( myRgn, winPtr->strucRgn, winPtr->strucRgn );		DisposeRgn ( myRgn );	}}// -----------------------------------------------------------------------------------------------// This function returns in which part of the window the user clicked.// -----------------------------------------------------------------------------------------------long HitMessage ( short varCode, WindowPeek winPtr, long param ){	Rect		boundRect,				contentRect,				titleBarRect,				growIconRect,				closeBoxRect,				zoomBoxRect,				stdStateRect;	// For zooming	Point		myPos;	FontInfo	MyFontData;		GetFontInfo ( &MyFontData );		myPos.v = HiWord(param);	myPos.h = LoWord(param);		GlobalContentRect ( winPtr, &contentRect );		CopyRect ( &contentRect, &boundRect );	InsetRect ( &boundRect, -6, -6);		if ( varCode != 1 && varCode != 2 && varCode != 3 )		boundRect.top += 5;		if ( varCode != 1 && varCode != 2 && varCode != 3 )	{		titleBarRect.bottom = boundRect.top;		titleBarRect.top = boundRect.top -kTitleBarHeight;		titleBarRect.left = boundRect.left;		titleBarRect.right = boundRect.right;	}	else		SetRect ( &titleBarRect, 0, 0, 0, 0 );		CopyRect ( &contentRect, &growIconRect );	growIconRect.left = growIconRect.right - kGrowBoxSize;	growIconRect.top = growIconRect.bottom - kGrowBoxSize;		CopyRect ( &titleBarRect, &closeBoxRect );		closeBoxRect.left += 10;	closeBoxRect.top += 4;	closeBoxRect.bottom -= 4;	closeBoxRect.right = closeBoxRect.bottom -closeBoxRect.top +closeBoxRect.left;		CopyRect ( &titleBarRect, &zoomBoxRect );		zoomBoxRect.right -= 10;	zoomBoxRect.top += 4;	zoomBoxRect.bottom -= 4;	zoomBoxRect.left = zoomBoxRect.right -zoomBoxRect.bottom +zoomBoxRect.top;		if ( winPtr->hilited && varCode != 1 && varCode != 2 && varCode != 3 )	{		if ( PtInRect( myPos, &closeBoxRect ) && winPtr->goAwayFlag)			return ( wInGoAway );				if ( PtInRect( myPos, &zoomBoxRect ) && winPtr->spareFlag )		{			CopyRect ( &( (**((WStateDataHandle) winPtr->dataHandle)).stdState ), &stdStateRect );			OffsetRect ( &stdStateRect, -stdStateRect.left, -stdStateRect.top );						if ( EqualRect ( &winPtr->port.portRect, &stdStateRect ) )				return ( wInZoomIn );			else				return ( wInZoomOut );		}	}		if ( PtInRect( myPos, &titleBarRect ) && varCode != 1 && varCode != 2 && varCode != 3 )		return ( wInDrag );		if ( winPtr->hilited && varCode != 1 && varCode != 2 && varCode != 3 )	{		if ( PtInRect( myPos, &growIconRect ) && !BitAnd( varCode, 4) )			return ( wInGrow );	}		if ( PtInRect( myPos, &contentRect ) )		return ( wInContent );		if ( PtInRect( myPos, &boundRect ) )	{		if ( varCode != 1 && varCode != 2 && varCode != 3 )			return ( wInDrag );		else			return ( wNoHit );	}}// -----------------------------------------------------------------------------------------------// This procedure draws the window frame and title.// -----------------------------------------------------------------------------------------------void DrawMessage ( short varCode, WindowPeek winPtr, Boolean hasColor ){	Rect		aRect,				titleBarRect,				boundsRect,				zoomBoxRect,				closeBoxRect,				tempRect;	RgnHandle	myErase,				svClip,				myClip,				tempRgn;	short		halfBox,				WindowWidth,				x;	Str255		windowName;	FontInfo	MyFontData;		myErase = NewRgn();	svClip = NewRgn();	myClip = NewRgn();	tempRgn = NewRgn();		GlobalContentRect ( winPtr, &aRect );		GetFontInfo ( &MyFontData );	// Get info for system font.		// Now erase everything that's not content region:	DiffRgn ( winPtr->strucRgn, winPtr->contRgn, myErase );	if (hasColor)	{		if (winPtr->hilited)			SetColor ( kFrameCol, kFrameCol, kFrameCol );		else			SetColor ( kDimFrameCol, kDimFrameCol, kDimFrameCol );		PaintRgn ( myErase );		if (winPtr->hilited)			SetColor ( kBlackCol, kBlackCol, kBlackCol );		else			SetColor ( kDimmedCol, kDimmedCol, kDimmedCol );	}	else		EraseRgn ( myErase );		InsetRect ( &aRect, -1, -1 );	FrameRect ( &aRect );		if ( varCode != 2 && varCode != 3 )	{		CopyRect ( &aRect, &boundsRect );		InsetRect ( &boundsRect, -5, -5 );				if ( varCode != 1 )		{			boundsRect.top += 5;			boundsRect.top -= kTitleBarHeight;		}				FrameRect ( &boundsRect );				if ( varCode != 1 )		{			CopyRect ( &boundsRect, &titleBarRect );			titleBarRect.bottom = titleBarRect.top +kTitleBarHeight +1;						CopyRect ( &titleBarRect, &closeBoxRect );			CopyRect ( &titleBarRect, &zoomBoxRect );						if ( !winPtr->goAwayFlag )				closeBoxRect.right = closeBoxRect.left +1;						if ( !winPtr->spareFlag )				zoomBoxRect.left = zoomBoxRect.right -1;						if ( !winPtr->hilited && winPtr->goAwayFlag )			{				closeBoxRect.left += 10;				closeBoxRect.top += 4;				closeBoxRect.bottom -= 4;				closeBoxRect.right = closeBoxRect.bottom -closeBoxRect.top +closeBoxRect.left;								if( ((closeBoxRect.bottom -closeBoxRect.top) /2) *2 == (closeBoxRect.bottom -closeBoxRect.top) )				{					closeBoxRect.bottom -= 1;					closeBoxRect.right -= 1;				}			}						if ( !winPtr->hilited && winPtr->spareFlag )			{				zoomBoxRect.right -= 10;				zoomBoxRect.top += 4;				zoomBoxRect.bottom -= 4;				zoomBoxRect.left = zoomBoxRect.right -zoomBoxRect.bottom +zoomBoxRect.top;									if( ((zoomBoxRect.bottom -zoomBoxRect.top) /2) *2 == (zoomBoxRect.bottom -zoomBoxRect.top) )				{					zoomBoxRect.bottom -= 1;					zoomBoxRect.left += 1;				}			}						if ( winPtr->hilited )			{				if (winPtr->spareFlag)				{					zoomBoxRect.right -= 10;					zoomBoxRect.top += 4;					zoomBoxRect.bottom -= 4;					zoomBoxRect.left = zoomBoxRect.right -zoomBoxRect.bottom +zoomBoxRect.top;										if( ((zoomBoxRect.bottom -zoomBoxRect.top) /2) *2 == (zoomBoxRect.bottom -zoomBoxRect.top) )					{						zoomBoxRect.bottom -= 1;						zoomBoxRect.left += 1;					}										if (hasColor)					{						CopyRect ( &zoomBoxRect, &tempRect );												SetColor ( kFrameCol, kFrameCol, kFrameCol );						PaintRect ( &tempRect );						SetColor ( kBlackCol, kBlackCol, kBlackCol );						DrawRectParts ( &tempRect, DR_TopLeft );						InsetRect ( &tempRect, 1, 1 );						tempRect.top += 1;						tempRect.left += 1;						SetColor ( kGadgetCol, kGadgetCol, kGadgetCol );						PaintRect ( &tempRect );						SetColor ( kBlackCol, kBlackCol, kBlackCol );						DrawRectParts ( &tempRect, DR_BotRight );						SetColor ( kBlackCol, kBlackCol, kBlackCol );												halfBox = (tempRect.right -tempRect.left) /2;						tempRect.right = tempRect.left + halfBox;						tempRect.bottom = tempRect.top + halfBox;												DrawRectParts ( &tempRect, DR_BotRight );					}					else					{						halfBox = ((zoomBoxRect.right -zoomBoxRect.left) /3) *2;												FrameRect ( &zoomBoxRect );						zoomBoxRect.right = zoomBoxRect.left + halfBox;						zoomBoxRect.bottom = zoomBoxRect.top + halfBox;												DrawRectParts ( &zoomBoxRect, DR_BotRight );					}				}								if (winPtr->goAwayFlag)				{					closeBoxRect.left += 10;					closeBoxRect.top += 4;					closeBoxRect.bottom -= 4;					closeBoxRect.right = closeBoxRect.bottom -closeBoxRect.top +closeBoxRect.left;										if( ((closeBoxRect.bottom -closeBoxRect.top) /2) *2 == (closeBoxRect.bottom -closeBoxRect.top) )					{						closeBoxRect.bottom -= 1;						closeBoxRect.right -= 1;					}										if (hasColor)					{						CopyRect ( &closeBoxRect, &tempRect );												SetColor ( kFrameCol, kFrameCol, kFrameCol );						PaintRect ( &tempRect );						SetColor ( kBlackCol, kBlackCol, kBlackCol );						DrawRectParts ( &tempRect, DR_TopLeft );						InsetRect ( &tempRect, 1, 1 );						tempRect.top += 1;						tempRect.left += 1;						SetColor ( kGadgetCol, kGadgetCol, kGadgetCol );						PaintRect ( &tempRect );						SetColor ( kBlackCol, kBlackCol, kBlackCol );						DrawRectParts ( &tempRect, DR_BotRight );						SetColor ( kBlackCol, kBlackCol, kBlackCol );					}					else						FrameRect ( &closeBoxRect );				}			}		} // Dialog exception				// Outter shades				CopyRect ( &boundsRect, &tempRect );		InsetRect ( &tempRect, 1, 1 );				if (hasColor)			if (winPtr->hilited)				SetColor ( kShadeCol, kShadeCol, kShadeCol );			else				SetColor ( kDimShadeCol, kDimShadeCol, kDimShadeCol );		else			PenPat ( &qd.gray );				DrawRectParts ( &tempRect, DR_BotRight );				if (hasColor)		{			SetColor ( kWhiteCol, kWhiteCol, kWhiteCol );			DrawRectParts ( &tempRect, DR_TopLeft );			if (winPtr->hilited)				SetColor ( kBlackCol, kBlackCol, kBlackCol );			else				SetColor ( kDimmedCol, kDimmedCol, kDimmedCol );		}		else			PenPat ( &qd.black );				// Inner shades				CopyRect ( &aRect, &tempRect );		InsetRect ( &tempRect, -1, -1 );				if (hasColor)			if (winPtr->hilited)				SetColor ( kShadeCol, kShadeCol, kShadeCol );			else				SetColor ( kDimShadeCol, kDimShadeCol, kDimShadeCol );		else			PenPat ( &qd.gray );				DrawRectParts ( &tempRect, DR_TopLeft );				if (hasColor)		{			SetColor ( kWhiteCol, kWhiteCol, kWhiteCol );			DrawRectParts ( &tempRect, DR_BotRight );			if (winPtr->hilited)				SetColor ( kBlackCol, kBlackCol, kBlackCol );			else				SetColor ( kDimmedCol, kDimmedCol, kDimmedCol );		}		else			PenPat ( &qd.black );				if ( varCode != 1 )		{			// Title						GetClip ( svClip );			CopyRect ( &titleBarRect, &tempRect );			tempRect.left = closeBoxRect.right +4;			tempRect.right = zoomBoxRect.left -4;			RectRgn ( myClip, &tempRect );						SectRgn ( svClip, myClip, myClip );			SetClip ( myClip );						GetWTitle ( (WindowPtr) winPtr, &windowName );			winPtr->titleWidth = StringWidth( &windowName );			WindowWidth = titleBarRect.right -titleBarRect.left;			if ( winPtr->hilited )			{				MoveTo ( titleBarRect.left + (WindowWidth /2) +1, titleBarRect.bottom +1 );				Move ( -(winPtr->titleWidth /2), -MyFontData.descent -kTitleNameBorder );				SetColor ( kWhiteCol, kWhiteCol, kWhiteCol );				DrawString ( &windowName );				SetColor ( kBlackCol, kBlackCol, kBlackCol );		// No dim, is only called when active			}			MoveTo ( titleBarRect.left + (WindowWidth /2), titleBarRect.bottom );			Move ( -(winPtr->titleWidth /2), -MyFontData.descent -kTitleNameBorder );			DrawString ( &windowName );						if ( winPtr->hilited )			{				RectRgn ( myClip, &titleBarRect );								CopyRect ( &closeBoxRect, &tempRect );				tempRect.left -= 1;				tempRect.right += 1;				RectRgn ( tempRgn, &tempRect );				DiffRgn ( myClip, tempRgn, myClip );								CopyRect ( &zoomBoxRect, &tempRect );				tempRect.left -= 1;				tempRect.right += 1;				RectRgn ( tempRgn, &tempRect );				DiffRgn ( myClip, tempRgn, myClip );								CopyRect ( &titleBarRect, &tempRect );				tempRect.left = titleBarRect.left + (WindowWidth /2) -(winPtr->titleWidth /2);				tempRect.right = tempRect.left +winPtr->titleWidth;				tempRect.left -= 5;				tempRect.right += 5;				RectRgn ( tempRgn, &tempRect );								DiffRgn ( myClip, tempRgn, myClip );				SectRgn ( svClip, myClip, myClip );				SetClip ( myClip );								if (hasColor)					SetColor ( kHighlightCol, kHighlightCol, kHighlightCol );				x = titleBarRect.top +4;				while ( x < (titleBarRect.bottom -4) ) {					MoveTo ( titleBarRect.left +5, x );					LineTo ( titleBarRect.right -6, x );					x += 2;				}				if (hasColor) {					SetColor ( kWhiteCol, kWhiteCol, kWhiteCol );					x = titleBarRect.top +4;					while ( x < (titleBarRect.bottom -4) ) {						MoveTo ( titleBarRect.left +5, x+1 );						LineTo ( titleBarRect.right -5, x+1 );						x += 2;					}				}				SetColor ( kBlackCol, kBlackCol, kBlackCol );			}						SetClip ( svClip );		} // Dialog exception	} // Rect/Shadow exception		if ( varCode == 3 )	{		if (hasColor)		SetColor ( kShadeCol, kShadeCol, kShadeCol );		PenSize ( 2, 2 );		CopyRect ( &aRect, &tempRect );		tempRect.right += 1;		tempRect.bottom += 1;		tempRect.left += 2;		tempRect.top += 2;		DrawRectParts ( &tempRect, DR_BotRight );		PenSize ( 1, 1 );	}		if (hasColor)		SetColor ( kBlackCol, kBlackCol, kBlackCol );		// For window Manager		DisposeRgn ( myErase );	DisposeRgn ( myClip );	DisposeRgn ( svClip );	DisposeRgn ( tempRgn );}// -----------------------------------------------------------------------------------------------// This procedure highlights the close box and unhilites it.// -----------------------------------------------------------------------------------------------void DrawGoAwayMessage ( short /*varCode*/, WindowPeek winPtr, Boolean /*hasColor*/ ){	Rect		aRect,				closeBoxRect,				titleBarRect;	FontInfo	MyFontData;		GetFontInfo ( &MyFontData );	GlobalContentRect ( winPtr, &aRect);		InsetRect ( &aRect, -6, -6);	aRect.top += 5;		titleBarRect.bottom = aRect.top;	titleBarRect.top = aRect.top -kTitleBarHeight;	titleBarRect.left = aRect.left;	titleBarRect.right = aRect.right;		CopyRect ( &titleBarRect, &closeBoxRect );		closeBoxRect.left += 10;	closeBoxRect.top += 4;	closeBoxRect.bottom -= 4;	closeBoxRect.right = closeBoxRect.bottom -closeBoxRect.top +closeBoxRect.left;		if( ((closeBoxRect.bottom -closeBoxRect.top) /2) *2 == (closeBoxRect.bottom -closeBoxRect.top) )	{		closeBoxRect.bottom -= 1;		closeBoxRect.right -= 1;	}		// Inset by 1. Since frame is 1 pt larger, we only need to do this on topLeft	closeBoxRect.left ++;	closeBoxRect.top ++;		InvertRect ( &closeBoxRect );}// -----------------------------------------------------------------------------------------------// This procedure highlights the zoom box and unhilites it.// -----------------------------------------------------------------------------------------------void DrawZoomMessage ( short /*varCode*/, WindowPeek winPtr, Boolean /*hasColor*/ ){	Rect		aRect,				zoomBoxRect,				titleBarRect;	FontInfo	MyFontData;					GetFontInfo ( &MyFontData );	GlobalContentRect ( winPtr, &aRect);		InsetRect ( &aRect, -6, -6);	aRect.top += 5;		titleBarRect.bottom = aRect.top;	titleBarRect.top = aRect.top -kTitleBarHeight;	titleBarRect.left = aRect.left;	titleBarRect.right = aRect.right;		CopyRect ( &titleBarRect, &zoomBoxRect );		zoomBoxRect.right -= 10;	zoomBoxRect.top += 4;	zoomBoxRect.bottom -= 4;	zoomBoxRect.left = zoomBoxRect.right -zoomBoxRect.bottom +zoomBoxRect.top;		if( ((zoomBoxRect.bottom -zoomBoxRect.top) /2) *2 == (zoomBoxRect.bottom -zoomBoxRect.top) )	{		zoomBoxRect.bottom -= 1;		zoomBoxRect.left += 1;	}		// Make it as large as the framed version	zoomBoxRect.right --;	zoomBoxRect.top ++;		InvertRect ( &zoomBoxRect );}// -----------------------------------------------------------------------------------------------// This procedure initializes the WStateData record and sets the flags of the window Record// depending on the variation code.// -----------------------------------------------------------------------------------------------void NewMessage ( short varCode, WindowPeek winPtr, long /*param*/ ){	WStateDataHandle	myWData;	Rect				aRect;		GlobalWRect ( winPtr, &aRect);		// WState Record for zooming		winPtr->dataHandle = NewHandleClear( sizeof( WStateData ) );		myWData = (WStateDataHandle) winPtr->dataHandle;	CopyRect ( &aRect, &(**myWData).userState );	CopyRect ( &qd.screenBits.bounds, &(**myWData).stdState );					// Close flag (for movable modal)		if ( varCode == 5 || varCode == 13 )	{		winPtr->goAwayFlag = false;	}		// Zoom flag		if ( BitAnd( varCode, 8) )	{		winPtr->spareFlag = true;	}	}// -----------------------------------------------------------------------------------------------// This procedure disposes tha data allocated on NewMessage.// -----------------------------------------------------------------------------------------------void DisposeMessage ( short /*varCode*/, WindowPeek winPtr, long /*param*/ ){	DisposHandle ( winPtr->dataHandle );}void GrowRectMessage ( short varCode, WindowPeek /*winPtr*/, Rect *aRect ){	Rect		boundsRect,				outterRect,				gBoxRect;	FontInfo	MyFontData;		GetFontInfo ( &MyFontData );	CopyRect ( aRect, &boundsRect);		InsetRect ( &boundsRect, -1, -1 );	FrameRect ( &boundsRect );		CopyRect ( &boundsRect, &outterRect );		InsetRect ( &outterRect, -5, -5 );	outterRect.top += 5;	outterRect.top -= kTitleBarHeight;	if (varCode != 1 && varCode != 2 && varCode != 3)		FrameRect ( &outterRect );		CopyRect ( &boundsRect, &gBoxRect );	gBoxRect.left = gBoxRect.right -kGrowBoxSize;	gBoxRect.top = gBoxRect.bottom -kGrowBoxSize;	FrameRect ( &gBoxRect );		MoveTo ( boundsRect.left +1, boundsRect.bottom -kGrowBoxSize );	LineTo ( boundsRect.right -kGrowBoxSize -1, boundsRect.bottom -kGrowBoxSize );	MoveTo ( boundsRect.right -kGrowBoxSize, boundsRect.top +1 );	LineTo ( boundsRect.right -kGrowBoxSize, boundsRect.bottom -kGrowBoxSize -1 );}// -----------------------------------------------------------------------------------------------// This procedure draws the grow box and the lines for the scroll bar regions.// -----------------------------------------------------------------------------------------------void GrowIconMessage ( short /*varCode*/, WindowPeek winPtr, long /*param*/, Boolean hasColor ){	Rect		aRect,				growIconRect,				smallGIRect,				tempRect;	RgnHandle	svClip;	GrafPtr		oldPort;		svClip = NewRgn();		GlobalContentRect ( winPtr, &aRect);	InsetRect ( &aRect, -1, -1);		CopyRect ( &aRect, &growIconRect );	growIconRect.left = growIconRect.right - kGrowBoxSize;	growIconRect.top = growIconRect.bottom - kGrowBoxSize;		growIconRect.right += 1;	growIconRect.bottom += 1;		SetClip ( winPtr->strucRgn );						// Set clip rgn	ClipAbove ( winPtr );								// Remove invisible parts		if (hasColor)	{		CopyRect ( &growIconRect, &tempRect );		tempRect.left += 1;		tempRect.top += 1;		if (winPtr->hilited)			SetColor ( kFrameCol, kFrameCol, kFrameCol );		else			SetColor ( kDimFrameCol, kDimFrameCol, kDimFrameCol );		PaintRect ( &tempRect );		if (winPtr->hilited)			SetColor ( kBlackCol, kBlackCol, kBlackCol );		else			SetColor ( kDimmedCol, kDimmedCol, kDimmedCol );	}	else		EraseRect ( &growIconRect );		CopyRect ( &growIconRect, &smallGIRect );		smallGIRect.bottom -= 1;	smallGIRect.right -= 1;		DrawRectParts ( &smallGIRect, DR_TopLeft );	GetPort(&oldPort);	SetPort((GrafPtr)winPtr);	MoveTo ( winPtr->port.portRect.right -kGrowBoxSize +1, winPtr->port.portRect.bottom -1);	LineTo ( winPtr->port.portRect.right -kGrowBoxSize +1, winPtr->port.portRect.top );	MoveTo ( winPtr->port.portRect.right -1, winPtr->port.portRect.bottom -kGrowBoxSize +1 );	LineTo ( winPtr->port.portRect.left, winPtr->port.portRect.bottom -kGrowBoxSize +1 );	SetPort(oldPort);		InsetRect ( &growIconRect, 1, 1 );	growIconRect.bottom += 1;	growIconRect.right += 1;		if (hasColor)	{		SetColor ( kWhiteCol, kWhiteCol, kWhiteCol );		DrawRectParts ( &growIconRect, DR_TopLeft );		if (winPtr->hilited)			SetColor ( kBlackCol, kBlackCol, kBlackCol );		else			SetColor ( kDimmedCol, kDimmedCol, kDimmedCol );	}		if (winPtr->hilited)	{		CopyRect ( &growIconRect, &tempRect );				tempRect.left += 4;		tempRect.top += 4;		tempRect.right -= 3;		tempRect.bottom -= 3;		EraseRect ( &tempRect );		if (hasColor)		{			SetColor ( kFrameCol, kFrameCol, kFrameCol );			PaintRect ( &tempRect );			SetColor ( kBlackCol, kBlackCol, kBlackCol );			DrawRectParts ( &tempRect, DR_TopLeft );			tempRect.top += 2;			tempRect.left += 2;			SetColor ( kGadgetCol, kGadgetCol, kGadgetCol );			PaintRect ( &tempRect );			SetColor ( kBlackCol, kBlackCol, kBlackCol );			DrawRectParts ( &tempRect, DR_BotRight );			SetColor ( kBlackCol, kBlackCol, kBlackCol );		}		else			FrameRect ( &tempRect );				CopyRect ( &growIconRect, &tempRect );				tempRect.left += 3;		tempRect.top += 3;		tempRect.right -= 7;		tempRect.bottom -= 7;		EraseRect ( &tempRect );		if (hasColor)		{			SetColor ( kFrameCol, kFrameCol, kFrameCol );			PaintRect ( &tempRect );			SetColor ( kBlackCol, kBlackCol, kBlackCol );			DrawRectParts ( &tempRect, DR_TopLeft );			tempRect.top += 2;			tempRect.left += 2;			SetColor ( kTopGrowCol, kTopGrowCol, kTopGrowCol );			PaintRect ( &tempRect );			SetColor ( kBlackCol, kBlackCol, kBlackCol );			DrawRectParts ( &tempRect, DR_BotRight );			SetColor ( kBlackCol, kBlackCol, kBlackCol );		}		else			FrameRect ( &tempRect );	}		if (hasColor)		SetColor ( kBlackCol, kBlackCol, kBlackCol );			// For window Mgr.		SetClip ( svClip );		DisposeRgn ( svClip );}// -----------------------------------------------------------------------------------------------// This procedure returns the rect of the window in global coordinates.// -----------------------------------------------------------------------------------------------void GlobalWRect ( WindowPeek winPtr, Rect *aRect ){	GrafPtr		svPort;	Point		tempPoint;			GetPort ( &svPort );	SetPort ( (GrafPtr) winPtr );		CopyRect ( &(winPtr->port.portRect), aRect );		SetPt ( &tempPoint, 0, 0 );	LocalToGlobal ( &tempPoint );		OffsetRect ( aRect, tempPoint.h, tempPoint.v );		SetPort ( svPort );}// -----------------------------------------------------------------------------------------------// This procedure returns the rect of the window based on the content region. This assures// compatibility to WindowShade and consorts.// -----------------------------------------------------------------------------------------------void GlobalContentRect ( WindowPeek winPtr, Rect *aRect ){	CopyRect ( &((**winPtr->contRgn).rgnBBox), aRect); // Copy region bounds}// -----------------------------------------------------------------------------------------------// This procedure sets the foreground color to an RGB value passed as three different shorts.// -----------------------------------------------------------------------------------------------void SetColor ( short redColor, short greenColor, short blueColor ){	RGBColor		myColor;		myColor.red = redColor;	myColor.green = greenColor;	myColor.blue = blueColor;		RGBForeColor ( &myColor );}// -----------------------------------------------------------------------------------------------// This function returns whether the screens the window is displayed on support 4 Bit or more.// -----------------------------------------------------------------------------------------------Boolean CheckColor ( WindowPeek winPtr ){	OSErr		theErr;	Boolean		hasColor;	GDHandle	aDevice;	long		theVal;		theErr = Gestalt( 'qdrw', &theVal );	hasColor = ( BitAnd( theVal, 1 ) != 0 );	aDevice = GetMaxDevice( &((**(winPtr->strucRgn)).rgnBBox) );	hasColor = (hasColor && ( (**((**aDevice).gdPMap)).pixelSize >= 4));		return (hasColor);}// -----------------------------------------------------------------------------------------------// This procedure sets the GrapfPort to the Color Window Manager's port, if hasColor is TRUE.// -----------------------------------------------------------------------------------------------void SetColorPort ( Boolean hasColor ){	CGrafPtr	aPort;	RgnHandle	myClip;	PenState	pnState;		if ( hasColor == true )	{		myClip = NewRgn();				GetCWMgrPort ( &aPort );				GetPenState ( &pnState );		GetClip ( myClip );				SetPort ( (GrafPtr) aPort );				SetPenState ( &pnState );		SetClip ( myClip );				DisposeRgn ( myClip );	}}// -----------------------------------------------------------------------------------------------// This procedure copies the data from one rect to the other.// -----------------------------------------------------------------------------------------------void CopyRect ( Rect *s, Rect *d){	(*d).left = (*s).left;	(*d).top = (*s).top;	(*d).right = (*s).right;	(*d).bottom = (*s).bottom;};// -----------------------------------------------------------------------------------------------// This procedure draws only the specified sides of a rect. It requires the enum at the beginning// Of this script.// -----------------------------------------------------------------------------------------------void DrawRectParts ( Rect *aRect, short choice ){	if ( BitAnd(choice, 1) ) {		MoveTo ( (*aRect).left, (*aRect).top );		LineTo ( (*aRect).right -1, (*aRect).top );	}	if ( BitAnd(choice, 2) ) {		MoveTo ( (*aRect).left, (*aRect).top );		LineTo ( (*aRect).left, (*aRect).bottom -1 );	}	if ( BitAnd(choice, 4) ) {		MoveTo ( (*aRect).left, (*aRect).bottom -1 );		LineTo ( (*aRect).right -1, (*aRect).bottom -1 );	}	if ( BitAnd(choice, 8) ) {		MoveTo ( (*aRect).right -1, (*aRect).top );		LineTo ( (*aRect).right -1, (*aRect).bottom -1 );	}};