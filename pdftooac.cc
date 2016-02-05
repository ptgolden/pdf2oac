#include <stdio.h>
#include <goo/GooString.h>
#include <PDFDoc.h>
#include <PDFDocFactory.h>
#include <Page.h>
#include <Annot.h>
#include <XRef.h>

void processPage(PDFDoc* doc, Page* page) {
	Annots *annots;
	Annot *annot;

	annots = page->getAnnots();

	for (int j = 0; j < annots->getNumAnnots(); ++j) {
		annot = annots->getAnnot(j);

		if (annot->getType() == Annot::typeStamp) {
			AnnotStamp *stamp = static_cast<AnnotStamp *>(annot);
			GooString *subject = stamp->getSubject();
			printf("%s\n", subject->getCString());
		}

		if (annot->getType() == Annot::typeUnderline) {
			AnnotTextMarkup *underline = static_cast<AnnotTextMarkup *>(annot);
			Object annotObj;
			Object obj1;

			const Ref ref = annot->getRef();
			doc->getXRef()->fetch(ref.num, ref.gen, &annotObj);

			annotObj.dictLookup("MarkupText", &obj1);
			printf("%s", obj1.getString()->getCString());

			annotObj.free();
			obj1.free();
		}
	}
}

int main(int argv, char *args[]) {
	PDFDoc *doc;
	Page *page;

	// Hard coded for now; in the future take from args
	GooString *filename  = new GooString("/home/patrick/Code/projects/annot/test.pdf");

	doc = PDFDocFactory().createPDFDoc(*filename, NULL, NULL);
	int pages = doc->getNumPages();

	page = doc->getPage(2);
	processPage(doc, page);

	delete filename;
	delete doc;
}
