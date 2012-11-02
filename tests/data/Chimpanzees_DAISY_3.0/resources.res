<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE resources PUBLIC "-//NISO//DTD resource 2005-1//EN"
"http://www.daisy.org/z3986/2005/resource-2005-1.dtd">
<resources xmlns="http://www.daisy.org/z3986/2005/resource/" version="2005-1">

	<!-- Escapable -->
	<scope nsuri="http://www.w3.org/2001/SMIL20/">
		<nodeSet id="ns001" select="//seq[@class='prodnote']">
			<resource xml:lang="en" id="resID000">
				<text>prodnote</text>
			</resource>
		</nodeSet>

	</scope>

	<!-- Skippable -->
	<scope nsuri="http://www.daisy.org/z3986/2005/ncx/">
		<nodeSet id="ns002" select="//smilCustomTest[@bookStruct='PAGE_NUMBER']">
			<resource xml:lang="en" id="resID003">
				<text>page</text>
			</resource>
		</nodeSet>
		<nodeSet id="ns003" select="//smilCustomTest[@bookStruct='OPTIONAL_PRODUCER_NOTE']">
			<resource xml:lang="en" id="resID004">
				<text>producer note</text>
			</resource>
		</nodeSet>

	</scope>
</resources>
