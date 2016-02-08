#include <stdio.h>
#include <raptor.h>
#include <goo/GooString.h>
#include <PDFDoc.h>
#include <PDFDocFactory.h>
#include <Page.h>
#include <Annot.h>
#include <XRef.h>
#include <string>
#include <unordered_map>



namespace NS {
	const std::string OA = "oa";
	const std::string RDF = "rdf";
	const std::string BIBO = "bibo";
}

namespace RDF {
	raptor_world *world = NULL;
	raptor_serializer* serializer = NULL;
	std::unordered_map<std::string, raptor_uri*> nsMap;

	void init() {
		world = raptor_new_world();
		serializer = raptor_new_serializer(world, "turtle");

		nsMap[NS::OA] = raptor_new_uri(world, (const unsigned char*)"http://www.w3.org/ns/oa#");
		raptor_serializer_set_namespace(serializer, nsMap[NS::OA], (const unsigned char*)"oa");

		nsMap[NS::BIBO] = raptor_new_uri(world, (const unsigned char*)"http://purl.org/ontology/bibo/");
		raptor_serializer_set_namespace(serializer, nsMap[NS::BIBO], (const unsigned char*)"bibo");

		nsMap[NS::RDF] = raptor_new_uri(world, raptor_rdf_namespace_uri);

		raptor_serializer_start_to_file_handle(serializer, NULL, stdout);
	}

	void freeAll() {
		raptor_free_uri(nsMap[NS::OA]);
		raptor_free_uri(nsMap[NS::BIBO]);
		raptor_free_uri(nsMap[NS::RDF]);

		raptor_serializer_serialize_end(RDF::serializer);
		raptor_free_serializer(serializer);
		raptor_free_world(world);
	}
}

raptor_term* make_annotation() {
	raptor_statement *triple;
	raptor_term *annotationTerm;
	raptor_uri *pURI;
	raptor_uri *oURI;

	annotationTerm = raptor_new_term_from_blank(RDF::world, (const unsigned char*)"annot");
	pURI = raptor_new_uri_from_uri_local_name(RDF::world, RDF::nsMap[NS::RDF],
			(const unsigned char*)"Type");
	oURI = raptor_new_uri_from_uri_local_name(RDF::world, RDF::nsMap[NS::OA],
			(const unsigned char*)"Annotation");


	triple = raptor_new_statement(RDF::world);
	triple->subject = annotationTerm;
	triple->predicate = raptor_new_term_from_uri(RDF::world, pURI);
	triple->object = raptor_new_term_from_uri(RDF::world, oURI);

	raptor_serializer_serialize_statement(RDF::serializer, triple);

	raptor_free_statement(triple);
	raptor_free_uri(pURI);
	raptor_free_uri(oURI);

	return annotationTerm;
}

void process_page(PDFDoc* doc, raptor_term* pdfTerm, int pageNumber) {
	Page *page;
	Annots *annots;
	Annot *annot;

	page = doc->getPage(pageNumber);
	annots = page->getAnnots();

	for (int j = 0; j < annots->getNumAnnots(); ++j) {
		annot = annots->getAnnot(j);

		if (annot->getType() == Annot::typeStamp) {
			AnnotStamp *stamp = static_cast<AnnotStamp *>(annot);
			GooString *subject = stamp->getSubject();
			// printf("%s\n", subject->getCString());
		}

		if (annot->getType() == Annot::typeUnderline) {
			AnnotTextMarkup *underline = static_cast<AnnotTextMarkup *>(annot);
			Object annotObj;
			Object obj1;

			const Ref ref = annot->getRef();
			doc->getXRef()->fetch(ref.num, ref.gen, &annotObj);

			annotObj.dictLookup("MarkupText", &obj1);
			// printf("%s", obj1.getString()->getCString());

			annotObj.free();
			obj1.free();
		}
	}
}

void add_pdf_term(raptor_term *pdf_term) {
	raptor_statement *triple;
	raptor_uri *pURI;
	raptor_uri *oURI;

	pURI = raptor_new_uri_from_uri_local_name(RDF::world, RDF::nsMap[NS::RDF],
			(const unsigned char*)"Type");
	oURI = raptor_new_uri_from_uri_local_name(RDF::world, RDF::nsMap[NS::BIBO],
			(const unsigned char*)"Document");

	triple = raptor_new_statement(RDF::world);
	triple->subject = pdf_term;
	triple->predicate = raptor_new_term_from_uri(RDF::world, pURI);
	triple->object = raptor_new_term_from_uri(RDF::world, oURI);

	raptor_serializer_serialize_statement(RDF::serializer, triple);

	raptor_free_statement(triple);
	raptor_free_uri(pURI);
	raptor_free_uri(oURI);
}

int main(int argv, char *args[]) {
	PDFDoc *doc;
	GooString *filename;
	raptor_term *pdf_term;
	raptor_term *oa_term;

	RDF::init();

	// Hard coded for now; in the future take from args
	filename  = new GooString("/home/patrick/Code/projects/annot/test.pdf");
	doc = PDFDocFactory().createPDFDoc(*filename, NULL, NULL);

	pdf_term = raptor_new_term_from_blank(RDF::world, (const unsigned char*)"pdf");

	add_pdf_term(pdf_term);
	raptor_serializer_flush(RDF::serializer);

	int pages = doc->getNumPages();
	process_page(doc, pdf_term, 2);

	// Clean up
	delete filename;
	delete doc;
	RDF::freeAll();
	raptor_free_term(pdf_term);
	raptor_free_term(oa_term);
}
