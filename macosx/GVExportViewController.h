/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at http://www.graphviz.org/
 *************************************************************************/

#import <Cocoa/Cocoa.h>

@interface GVExportViewController : NSViewController
{
	NSSavePanel *_panel;
	NSURL *_url;
	NSDictionary *_formatRender;
	NSString *_render;
}

@property(readonly) NSArray *formatRenders;
@property(readonly) NSString *device;
@property(retain) NSURL *URL;
@property(retain) NSDictionary *formatRender;
@property(retain) NSString *render;

- (id)init;

- (void)beginSheetModalForWindow:(NSWindow *)window modalDelegate:(id)modalDelegate didEndSelector:(SEL)selector;

@end
