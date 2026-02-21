/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at http://www.graphviz.org/
 *************************************************************************/

#import "GVWindowController.h"
#import "GVZGraph.h"
#import "GVDocument.h"

@implementation GVWindowController

- (id)init
{
	if (self = [super initWithWindowNibName: @"Document"])
		;
	return self;
}

- (void)setDocument: (NSDocument *)document
{
			NSNotificationCenter* defaultCenter = [NSNotificationCenter defaultCenter];
	
	GVDocument *oldDocument = [self document];
	if (oldDocument)
		[defaultCenter removeObserver:self name:@"GVGraphDocumentDidChange" object:oldDocument];
	[super setDocument:document];
	[defaultCenter addObserver:self selector:@selector(graphDocumentDidChange:) name:@"GVGraphDocumentDidChange" object:document];
}

- (void)awakeFromNib
{
	[self graphDocumentDidChange:nil];
	
	/* if window is not at standard size, make it standard size */
	NSWindow *window = [self window];
	if (![window isZoomed])
		[window zoom:self];

	[documentView setDelegate:self];
	[documentView setBackgroundColor:[NSColor textBackgroundColor]];
	[documentView setAutoScales:YES];
	if (@available(macOS 10.14, *)) {
		[documentView enablePageShadows:NO];
	}
	
	// SF Symbols are unavailable before 11.0.
	// All other fallback images are provided in the app,
	// but for Attributes button a system image was used
	if (attributesToolbarItem.image == nil) {
		attributesToolbarItem.image = [NSImage imageNamed:@"NSInfo"];
	}
}

- (void)graphDocumentDidChange:(NSNotification*)notification
{
	/* whenever the graph changes, rerender its PDF and display that */
	GVDocument *document = [self document];
	if ([document respondsToSelector:@selector(graph)])
		[documentView setDocument:[[[PDFDocument alloc] initWithData:[[document graph] renderWithFormat:@"pdf:quartz"]] autorelease]];
}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)defaultFrame
{
	/* standard size for zooming is whatever will fit the content exactly */
	NSRect currentFrame = [window frame];
	NSRect standardFrame = [window frameRectForContentRect:[[documentView documentView] bounds]];
	standardFrame.origin.x = currentFrame.origin.x;
	standardFrame.origin.y = currentFrame.origin.y + currentFrame.size.height - standardFrame.size.height;
	return standardFrame;
}

- (IBAction)actualSizeView:(id)sender
{
	[documentView setScaleFactor:1.0];
}

- (IBAction)zoomInView:(id)sender
{
	[documentView zoomIn:sender];
}

- (IBAction)zoomOutView:(id)sender
{
	[documentView zoomOut:sender];
}

- (IBAction)zoomToFitView:(id)sender
{
	[documentView setAutoScales:YES];
}

- (IBAction)search:(id)sender
{
	[self searchNextBackwards:NO];
}

- (void)performTextFinderAction:(id)sender
{
    [self.searchField becomeFirstResponder];
}

- (void)searchNextBackwards:(BOOL)backwards {
	[documentView setHighlightedSelections:nil];
	NSString *searchString = self.searchField.stringValue;
	NSArray<PDFSelection*>* selection = [documentView.document findString:searchString withOptions:NSCaseInsensitiveSearch];
	[documentView setHighlightedSelections:selection];

	NSStringCompareOptions options = NSCaseInsensitiveSearch;
	if (backwards)
		options |= NSBackwardsSearch;
	PDFSelection* nextSelection = [documentView.document findString:searchString fromSelection:documentView.currentSelection withOptions:options];
	[documentView setCurrentSelection:nextSelection animate:YES];
	if (nextSelection != nil)
			[documentView goToSelection:nextSelection];
}

- (void)searchNext:(id)sender {
	[self searchNextBackwards:NO];
}

- (void)searchPrevious:(id)sender {
	[self searchNextBackwards:YES];
}

- (IBAction)printGraphDocument:(id)sender
{
	[documentView printWithInfo:[[self document] printInfo] autoRotate:NO pageScaling:kPDFPrintPageScaleDownToFit];
}

- (void)PDFViewWillClickOnLink:(PDFView *)sender withURL:(NSURL *)URL
{
	NSURL* baseURL = [[self document] fileURL];
	NSURL* targetURL = [NSURL URLWithString:[URL absoluteString] relativeToURL:baseURL];
	[[NSWorkspace sharedWorkspace] openURL:targetURL];
}

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem
{
	/* validate toolbar or menu items */
	if ([anItem action] == @selector(actualSizeView:))
		return YES;
	else if ([anItem action] == @selector(zoomInView:))
		return [documentView canZoomIn];
	else if ([anItem action] == @selector(zoomOutView:))
		return [documentView canZoomOut];
	else if ([anItem action] == @selector(zoomToFitView:))
		return YES;
	else if ([anItem action] == @selector(printGraphDocument:))
		return YES;
	else if ([anItem action] == @selector(performTextFinderAction:))
			return YES;
	else if ([anItem action] == @selector(searchNext:))
			return ![[self.searchField stringValue] isEqualToString:@""];
	else if ([anItem action] == @selector(searchPrevious:))
			return ![[self.searchField stringValue] isEqualToString:@""];
	else
		return NO;
}
- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:@"GVGraphDocumentDidChange" object:[self document]];
    [super dealloc];
}


@end
