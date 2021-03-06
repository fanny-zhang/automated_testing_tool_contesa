<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- English revision: 1.4 -->
<xsl:stylesheet version="1.0"
              xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output 
  method="xml"
  encoding="EUC-KR"
  indent="no"
  doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
  doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
/>

<!-- Read the localized messages from the specified language file -->
<xsl:variable name="messages" select="document('lang/ko.xml')/messages"/>

<!-- some meta information have to be passed to the transformation -->
<xsl:variable name="output-encoding" select="'EUC-KR'" />
<xsl:variable name="is-chm" select="false()" />

<!-- Now get the real guts of the stylesheet -->
<xsl:include href="xsl/common.xsl"/>

</xsl:stylesheet>
