/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at http://www.graphviz.org/
 *************************************************************************/

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <Quartz/Quartz.h>

#if !defined(MAC_OS_X_VERSION_10_13) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_13
@protocol PDFViewDelegate <NSObject> @end
#endif

@interface GVWindowController : NSWindowController <NSUserInterfaceValidations, PDFViewDelegate>
{
	IBOutlet PDFView *documentView;
}
@property (assign) IBOutlet NSSearchField *searchField;

- (id)init;
- (void)setDocument: (NSDocument *)document;
- (void)awakeFromNib;

- (void)graphDocumentDidChange:(NSNotification*)notification;
- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)defaultFrame;

- (IBAction)actualSizeView:(id)sender;
- (IBAction)zoomInView:(id)sender;
- (IBAction)zoomOutView:(id)sender;
- (IBAction)zoomToFitView:(id)sender;

- (IBAction)printGraphDocument:(id)sender;

- (void)performTextFinderAction:(id)sender;

- (void)PDFViewWillClickOnLink:(PDFView *)sender withURL:(NSURL *)URL;

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;

- (void)dealloc;
@end
