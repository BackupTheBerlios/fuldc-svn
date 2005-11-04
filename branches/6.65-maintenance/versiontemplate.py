from xml.dom import minidom
import os
import sys

#make sure relative paths work from the vc7.1 dir too
os.chdir(sys.argv[0][0: sys.argv[0].rfind("\\")])

ENTRIESSUBPATH = ".svn/entries"
TEMPLATE = "client/versiontemplate.h"
TARGET = "client/version.h"

DEBUG = True

class SVNReport:

	def __init__(self):
		self.revision = 0
		self.data = {}
	
	def walkrevision(self,root):

		for item in os.listdir(root):
			itempath = os.path.join(root, item)
			if os.path.isdir(itempath):
				if item == ".svn":
					self.findrevision(itempath)
				else:
					self.walkrevision(itempath)
			else:
				continue


	def findrevision(self, itempath):
		itempath = os.path.join(itempath,"entries")
		if not os.path.exists(itempath):
			return
		xmldom = minidom.parse(itempath)
		xmlEntries = xmldom.firstChild
		for node in xmlEntries.childNodes:
			if node.nodeType == node.TEXT_NODE:
				continue
			elif node.nodeType == node.ELEMENT_NODE and node.attributes.has_key('revision'):
				if self.revision > int(node.attributes["revision"].value): continue
				self.revision = int(node.attributes["revision"].value)
				d = {}
				for key in node.attributes.keys():
					d[key] = node.attributes[key].value
				self.data = d
		xmldom.unlink()


if __name__ == "__main__":
	s = SVNReport()
	s.walkrevision(".")
	versiontemplate = open(TEMPLATE,'r').read()
	print "Getting SVN Information:"
	for key,value in s.data.iteritems():
		versiontemplate = versiontemplate.replace("$%s" % key, value)
	print "Revision %s, checked in by %s" % (s.data.get("revision",0),s.data.get("last-author","Unknown"))
	print "Checking if update of version.h is needed"
	if not os.path.exists(TARGET):
		open(TARGET,'w').write(versiontemplate)
		print "Updates version.h from template file"
	elif open(TARGET,'r').read() != versiontemplate:
		print "Updates version.h from template file"
		open(TARGET,'w').write(versiontemplate)
	else:
		print "No changes required in version.h."