/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include "config.h"

#include "VisioText.h"

#include <gvc/gvcjob.h>
#include <gvc/gvio.h>
#include <cstdlib>
#include <string.h>

#include <common/utils.h>

namespace Visio
{
	static const float INCHES_PER_POINT = 1.0 / 72.0;

	Char::Char(double size, unsigned char red, unsigned char green, unsigned char blue):
		_size(size),
		_red(red),
		_green(green),
		_blue(blue)
	{
	}
	
	void Char::Print(GVJ_t* job) const
	{
		gvputs(job, "<Char>\n");
		gvprintf(job, "<Color>#%02X%02X%02X</Color>\n", _red, _green, _blue);
		gvprintf(job, "<Size>%f</Size>\n", _size * job->scale.x * INCHES_PER_POINT);	/* scale font size, VDX uses inches */
		gvputs(job, "</Char>\n");
	}
	
	Para::Para(HorzAlign horzAlign):
		_horzAlign(horzAlign)
	{
	}
	
	void Para::Print(GVJ_t* job) const
	{
		gvputs(job, "<Para>\n");
		gvprintf(job, "<HorzAlign>%d</HorzAlign>\n", _horzAlign);
		gvputs(job, "</Para>\n");
	}
	
	Run::Run(boxf bounds, const char* text):
		_bounds(bounds),
		_text(text)	/* copy text */
	{
	}
	
	boxf Run::GetBounds() const
	{
		return _bounds;
	}
	
	void Run::Print(GVJ_t* job, unsigned int index) const
	{
		// para mark + char mark + actual text
		gvprintf(job, "<pp IX='%u'/><cp IX='%u'/>", index, index);
		gvputs_xml(job, _text.c_str());
		gvputs(job, "\n");
	}
	
	Text Text::CreateText(GVJ_t* job, pointf p, textspan_t* span)
	{
		Para::HorzAlign horzAlign;
		
		/* compute text bounding box and VDX horizontal align */
		boxf bounds;
		bounds.LL.y = p.y + span->yoffset_centerline;
		bounds.UR.y = p.y + span->yoffset_centerline + span->size.y;
		double width = span->size.x;
		switch (span->just)
		{
			case 'r':
				horzAlign = Para::horzRight;
				bounds.LL.x = p.x - width;
				bounds.UR.x = p.x;
				break;
			case 'l':
				horzAlign = Para::horzLeft;
				bounds.LL.x = p.x;
				bounds.UR.x = p.x + width;
				break;
			case 'n':
			default:
				horzAlign = Para::horzCenter;
				bounds.LL.x = p.x - width / 2.0;
				bounds.UR.x = p.x + width / 2.0;
				break;
		}
		
		return Text(
			Para(
				horzAlign),
			Char(
				span->font->size,
				job->obj->pencolor.u.rgba[0],
				job->obj->pencolor.u.rgba[1],
				job->obj->pencolor.u.rgba[2]),
			Run(
				bounds,
				span->str));
	}
	
	Text::Text(const Para &para, const Char &chars, const Run &run):
		_para(para),
		_chars(chars),
		_run(run)
	{
	}
	
	boxf Text::GetBounds() const
	{
		return _run.GetBounds();
	}
	
	void Text::Print(GVJ_t* job) const
	{
		_para.Print(job);
		_chars.Print(job);
	}
	
	void Text::PrintRun(GVJ_t* job, unsigned int index) const
	{
		_run.Print(job, index);
	}

	Hyperlink::Hyperlink(const char* description, const char* address,
		const char* frame):
		_description(description),
		_address(address),
		_frame(frame)
	{
	}
	
	/* output the hyperlink */
	void Hyperlink::Print(GVJ_t* job, unsigned int id, bool isDefault) const
	{
		gvprintf(job, "<Hyperlink ID='%u'>\n", id);
		gvprintf(job, "<Description>%s</Description>\n", _description.c_str());
		gvprintf(job, "<Address>%s</Address>\n", _address.c_str());
		gvprintf(job, "<Frame>%s</Frame>\n", _frame.c_str());
		if (isDefault)
			gvputs(job, "<Default>1</Default>\n");
		gvputs(job, "</Hyperlink>\n");
	}
	
}
