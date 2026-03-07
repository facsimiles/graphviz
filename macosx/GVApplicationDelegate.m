/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at http://www.graphviz.org/
 *************************************************************************/

#import "GVApplicationDelegate.h"
#import "GVAttributeInspectorController.h"

@implementation GVApplicationDelegate

- (IBAction)showAttributeInspector:(id)sender
{
	if (!_attributeInspectorController)
		_attributeInspectorController = [[GVAttributeInspectorController alloc] init];
	[_attributeInspectorController showWindow:sender];
}

- (BOOL)applicationOpenUntitledFile:(NSApplication*)theApplication
{
	if (!_applicationStarted)
		[[NSDocumentController sharedDocumentController] openDocument:self];
	return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification
{
	_applicationStarted = YES;
}

@end
