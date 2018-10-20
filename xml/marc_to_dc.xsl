<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:marc="http://www.loc.gov/MARC21/slim">
	<xsl:output method="xml" indent="yes"/>
	<xsl:template match="/">
		<records>
			<xsl:for-each select="//marc:record">

				<xsl:variable name="leader" select="marc:leader"/>
				<xsl:variable name="leader6" select="substring($leader,7,1)"/>
				<xsl:variable name="leader7" select="substring($leader,8,1)"/>
				<xsl:variable name="leader19" select="substring($leader,20,1)"/>
				<xsl:variable name="controlField008" select="marc:controlfield[@tag='008']"/>
				<xsl:variable name="typeOf008">
					<xsl:choose>
						<xsl:when test="$leader6='a'">
							<xsl:choose>
								<xsl:when test="$leader7='a' or $leader7='c' or $leader7='d' or $leader7='m'">BK</xsl:when>
								<xsl:when test="$leader7='b' or $leader7='i' or $leader7='s'">SE</xsl:when>
							</xsl:choose>
						</xsl:when>
						<xsl:when test="$leader6='t'">BK</xsl:when>
						<xsl:when test="$leader6='p'">MM</xsl:when>
						<xsl:when test="$leader6='m'">CF</xsl:when>
						<xsl:when test="$leader6='e' or $leader6='f'">MP</xsl:when>
						<xsl:when test="$leader6='g' or $leader6='k' or $leader6='o' or $leader6='r'">VM</xsl:when>
						<xsl:when test="$leader6='c' or $leader6='d' or $leader6='i' or $leader6='j'">MU</xsl:when>
					</xsl:choose>
				</xsl:variable>

				<oai_dc:dc>
					<xsl:for-each select="marc:datafield[@tag=100]|marc:datafield[@tag=110]|marc:datafield[@tag=111]">
						<dc:creator>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abcdefgjklnpqtu468</xsl:with-param>
							</xsl:call-template>
						</dc:creator>
					</xsl:for-each>

					<xsl:for-each select="marc:datafield[@tag=700]|marc:datafield[@tag=710]|marc:datafield[@tag=711]">
						<dc:contributor>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abcdefghjklmnopqrstux468</xsl:with-param>
							</xsl:call-template>
						</dc:contributor>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=245]">
						<dc:title>
							<xsl:variable name="title">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">abcfghknps68</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:value-of select="replace(replace($title, '\[\[', ''), '\]\]', '')" />
						</dc:title>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=242]">
						<dc:title>
							<xsl:variable name="title">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">abchnpy68</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:value-of select="replace(replace($title, '\[\[', ''), '\]\]', '')" />
						</dc:title>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=246]">
						<dc:title>
							<xsl:variable name="title">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">abfghinp568</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:value-of select="replace(replace($title, '\[\[', ''), '\]\]', '')" />
						</dc:title>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=730]|marc:datafield[@tag=740]">
						<dc:title>
							<xsl:variable name="title">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">abfghinp568</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:value-of select="replace(replace($title, '\[\[', ''), '\]\]', '')" />
						</dc:title>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=260]">
						<xsl:variable name="pub">
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abefg68</xsl:with-param>
							</xsl:call-template>
						</xsl:variable>
						<xsl:variable name="pub2">
							<xsl:call-template name="chopPunctuation">
								<xsl:with-param name="chopString" select="$pub"/>
							</xsl:call-template>
						</xsl:variable>
						<xsl:if test="$pub2 != ''">
							<dc:publisher>
								<xsl:value-of select="$pub2"/>
								<xsl:if test="contains($pub2, '[') and not(contains($pub2, ']'))">
									<xsl:text>]</xsl:text>
								</xsl:if>
							</dc:publisher>
						</xsl:if>
					</xsl:for-each>

					<!-- get MARC-encoded dates from the 008 header field -->
					<!--
					<xsl:variable name="dataField260c">
						<xsl:call-template name="chopPunctuation">
							<xsl:with-param name="chopString" select="marc:datafield[@tag=260]/marc:subfield[@code='c']"/>
						</xsl:call-template>
					</xsl:variable>
					<xsl:variable name="controlField008-7-10" select="normalize-space(substring($controlField008, 8, 4))"/>
					<xsl:variable name="controlField008-11-14" select="normalize-space(substring($controlField008, 12, 4))"/>
					<xsl:variable name="controlField008-6" select="normalize-space(substring($controlField008, 7, 1))"/>

					<xsl:if test="($controlField008-6='e' or $controlField008-6='p' or $controlField008-6='r' or $controlField008-6='s' or $controlField008-6='t')">
						<xsl:if test="$controlField008-7-10 and ($controlField008-7-10 != $dataField260c)">
							<dc:date encoding="marc">
								<xsl:value-of select="$controlField008-7-10"/>
							</dc:date>
						</xsl:if>
					</xsl:if>

					<xsl:if test="$controlField008-6='c' or $controlField008-6='d' or $controlField008-6='i' or $controlField008-6='k' or $controlField008-6='m' or $controlField008-6='u'">
						<xsl:if test="$controlField008-7-10">
							<dc:date encoding="marc" point="start">
								<xsl:value-of select="$controlField008-7-10"/>
							</dc:date>
						</xsl:if>
					</xsl:if>

					<xsl:if test="$controlField008-6='c' or $controlField008-6='d' or $controlField008-6='i' or $controlField008-6='k' or $controlField008-6='m' or $controlField008-6='u'">
						<xsl:if test="$controlField008-11-14">
							<dc:date encoding="marc" point="end">
								<xsl:value-of select="$controlField008-11-14"/>
							</dc:date>
						</xsl:if>
					</xsl:if>

					<xsl:if test="$controlField008-6='q'">
						<xsl:if test="$controlField008-7-10">
							<dc:date encoding="marc" point="start" qualifier="questionable">
								<xsl:value-of select="$controlField008-7-10"/>
							</dc:date>
						</xsl:if>
					</xsl:if>
					<xsl:if test="$controlField008-6='q'">
						<xsl:if test="$controlField008-11-14">
							<dc:date encoding="marc" point="end" qualifier="questionable">
								<xsl:value-of select="$controlField008-11-14"/>
							</dc:date>
						</xsl:if>
					</xsl:if>
					-->

					<!-- // end get date from header -->

					<xsl:for-each select="marc:datafield[@tag=260]/marc:subfield[@code='c']">
						<dc:date>
							<xsl:variable name="dat">
								<xsl:call-template name="chopPunctuation">
									<xsl:with-param name="chopString" select="."/>
								</xsl:call-template>
							</xsl:variable>
							<xsl:if test="contains($dat, ']') and not(contains($dat, '['))">
								<xsl:text>[</xsl:text>
							</xsl:if>
							<xsl:value-of select="$dat"/>
						</dc:date>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=300]">
						<dc:format>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abcefg368</xsl:with-param>
							</xsl:call-template>
						</dc:format>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=520]">
						<dc:description>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abu368</xsl:with-param>
							</xsl:call-template>
						</dc:description>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=440]">
						<dc:relation>
							<xsl:text>Part of </xsl:text>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">anpvx68</xsl:with-param>
							</xsl:call-template>
						</dc:relation>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=773]">
						<dc:relation>
							<xsl:text>Part of </xsl:text>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abdghikmnoprstuwxyz367</xsl:with-param>
							</xsl:call-template>
						</dc:relation>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=830]">
						<dc:relation>
							<xsl:text>Part of </xsl:text>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">adfghklmnoprstv68</xsl:with-param>
							</xsl:call-template>
						</dc:relation>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=510]">
						<dc:relation>
							<xsl:text>Referenced by </xsl:text>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abcx368</xsl:with-param>
							</xsl:call-template>
						</dc:relation>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=500]">
						<dc:description>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">a3658</xsl:with-param>
							</xsl:call-template>
						</dc:description>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=505]">
						<dc:description>
							<xsl:choose>
								<xsl:when test="@ind1=0">
									<xsl:text>Contents: </xsl:text>
								</xsl:when>
								<xsl:when test="@ind1=1">
									<xsl:text>Incomplete contents: </xsl:text>
								</xsl:when>
								<xsl:when test="@ind1=2">
									<xsl:text>Partial contents: </xsl:text>
								</xsl:when>
								<xsl:otherwise></xsl:otherwise>
							</xsl:choose>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">agrtu68</xsl:with-param>
							</xsl:call-template>
						</dc:description>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=590]">
						<dc:description>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">a</xsl:with-param>
							</xsl:call-template>
						</dc:description>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=545]">
						<dc:description>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abu68</xsl:with-param>
							</xsl:call-template>
						</dc:description>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=600]|marc:datafield[@tag=610]|marc:datafield[@tag=611]|marc:datafield[@tag=630]|marc:datafield[@tag=650]|marc:datafield[@tag=690]">
						<xsl:choose>
							<xsl:when test="@tag=690 and marc:subfield[@code='9']"></xsl:when>
							<xsl:otherwise>
								<dc:subject>
									<xsl:variable name="sub1">
										<xsl:call-template name="subfieldSelect">
											<xsl:with-param name="codes">abcdefghjlkmnopqrstu</xsl:with-param>
										</xsl:call-template>
									</xsl:variable>
									<xsl:variable name="sub2">
										<xsl:call-template name="subfieldSelect">
											<xsl:with-param name="codes">vxyz</xsl:with-param>
											<xsl:with-param name="delimeter">
												<xsl:text> -- </xsl:text>
											</xsl:with-param>
										</xsl:call-template>
									</xsl:variable>
									<xsl:value-of select="$sub1"/>
									<xsl:if test="string-length($sub1) &gt; 0 and string-length($sub2) &gt; 0">
										<xsl:text> -- </xsl:text>
									</xsl:if>
									<xsl:value-of select="$sub2"/>
								</dc:subject>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:for-each>

					<xsl:for-each select="marc:datafield[@tag=651]">
						<dc:coverage>
							<xsl:variable name="sub1">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">abcdefghjlkmnopqrstu</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:variable name="sub2">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">vxyz</xsl:with-param>
									<xsl:with-param name="delimeter">
										<xsl:text> -- </xsl:text>
									</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:value-of select="$sub1"/>
							<xsl:if test="string-length($sub1) &gt; 0 and string-length($sub2) &gt; 0">
								<xsl:text> -- </xsl:text>
							</xsl:if>
							<xsl:value-of select="$sub2"/>
						</dc:coverage>
					</xsl:for-each>

					<xsl:for-each select="marc:datafield[@tag=752]">
						<dc:coverage>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abcdfg</xsl:with-param>
								<xsl:with-param name="delimeter">
									<xsl:text>-</xsl:text>
								</xsl:with-param>
							</xsl:call-template>
						</dc:coverage>
					</xsl:for-each>

					<xsl:for-each select="marc:datafield[@tag=655]">
						<dc:type>
							<!--
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abcvxyz3568</xsl:with-param>
							</xsl:call-template>
							-->
							<xsl:variable name="sub1">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">abc3568</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:variable name="sub2">
								<xsl:call-template name="subfieldSelect">
									<xsl:with-param name="codes">vxyz</xsl:with-param>
									<xsl:with-param name="delimeter">
										<xsl:text> -- </xsl:text>
									</xsl:with-param>
								</xsl:call-template>
							</xsl:variable>
							<xsl:value-of select="$sub1"/>
							<xsl:if test="string-length($sub1) &gt; 0 and string-length($sub2) &gt; 0">
								<xsl:text> -- </xsl:text>
							</xsl:if>
							<xsl:value-of select="$sub2"/>
						</dc:type>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=796] |marc:datafield[@tag=797]">
						<xsl:choose>
							<xsl:when test="contains(marc:subfield[@code='9'], 'Imprint')"></xsl:when>
							<xsl:otherwise>
								<dc:contributor>
									<xsl:for-each select="marc:subfield[@code='9']">
										<xsl:value-of select="text()"/>
										<xsl:choose>
											<xsl:when test="substring(., string-length(.), 1) != ':'">
												<xsl:text>: </xsl:text>
											</xsl:when>
											<xsl:otherwise>
												<xsl:text> </xsl:text>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:for-each>
									<xsl:call-template name="subfieldSelect">
										<xsl:with-param name="codes">abcqde</xsl:with-param>
									</xsl:call-template>
								</dc:contributor>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=852]">
						<dc:identifier>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">abchj</xsl:with-param>
							</xsl:call-template>
						</dc:identifier>
					</xsl:for-each>
					<xsl:for-each select="marc:datafield[@tag=850]">
						<dc:identifier>
							<xsl:call-template name="subfieldSelect">
								<xsl:with-param name="codes">aq</xsl:with-param>
							</xsl:call-template>
						</dc:identifier>
					</xsl:for-each>
				</oai_dc:dc>
			</xsl:for-each>
		</records>
	</xsl:template>

	<xsl:template name="subfieldSelect">
		<xsl:param name="codes">abcdefghijklmnopqrstuvwxyz123456789</xsl:param>
		<xsl:param name="delimeter">
			<xsl:text> </xsl:text>
		</xsl:param>
		<xsl:variable name="str">
			<xsl:for-each select="marc:subfield">
				<xsl:if test="contains($codes, @code)">
					<xsl:value-of select="text()"/>
					<xsl:value-of select="$delimeter"/>
				</xsl:if>
			</xsl:for-each>
		</xsl:variable>
		<xsl:value-of select="substring($str,1,string-length($str)-string-length($delimeter))"/>
	</xsl:template>

	<xsl:template name="chopPunctuation">
		<xsl:param name="chopString"/>
		<xsl:param name="punctuation">
			<xsl:text>.:,;/ </xsl:text>
		</xsl:param>
		<xsl:variable name="length" select="string-length($chopString)"/>
		<xsl:choose>
			<xsl:when test="$length=0"/>
			<xsl:when test="contains($punctuation, substring($chopString,$length,1))">
				<xsl:call-template name="chopPunctuation">
					<xsl:with-param name="chopString" select="substring($chopString,1,$length - 1)"/>
					<xsl:with-param name="punctuation" select="$punctuation"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="not($chopString)"/>
			<xsl:otherwise>
				<xsl:value-of select="$chopString"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="chopPunctuationFront">
		<xsl:param name="chopString"/>
		<xsl:variable name="length" select="string-length($chopString)"/>
		<xsl:choose>
			<xsl:when test="$length=0"/>
			<xsl:when test="contains('.:,;/[ ', substring($chopString,1,1))">
				<xsl:call-template name="chopPunctuationFront">
					<xsl:with-param name="chopString" select="substring($chopString,2,$length - 1)"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="not($chopString)"/>
			<xsl:otherwise>
				<xsl:value-of select="$chopString"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet><!-- Stylus Studio meta-information - (c) 2004-2009. Progress Software Corporation. All rights reserved.

<metaInformation>
	<scenarios>
		<scenario default="no" name="DickersonDags" userelativepaths="yes" externalpreview="no" url="DickersonDags.xml" htmlbaseurl="" outputurl="" processortype="saxon8" useresolver="no" profilemode="0" profiledepth="" profilelength="" urlprofilexml=""
		          commandline="" additionalpath="" additionalclasspath="" postprocessortype="none" postprocesscommandline="" postprocessadditionalpath="" postprocessgeneratedext="" validateoutput="no" validator="internal" customvalidator="">
			<advancedProp name="bSchemaAware" value="false"/>
			<advancedProp name="xsltVersion" value="2.0"/>
			<advancedProp name="schemaCache" value="||"/>
			<advancedProp name="iWhitespace" value="0"/>
			<advancedProp name="bWarnings" value="true"/>
			<advancedProp name="bXml11" value="false"/>
			<advancedProp name="bUseDTD" value="false"/>
			<advancedProp name="bXsltOneIsOkay" value="true"/>
			<advancedProp name="bTinyTree" value="true"/>
			<advancedProp name="bGenerateByteCode" value="false"/>
			<advancedProp name="bExtensions" value="true"/>
			<advancedProp name="iValidation" value="0"/>
			<advancedProp name="iErrorHandling" value="fatal"/>
			<advancedProp name="sInitialTemplate" value=""/>
			<advancedProp name="sInitialMode" value=""/>
		</scenario>
		<scenario default="yes" name="SelectedMarc" userelativepaths="no" externalpreview="no" url="file:///c:/projects/lcp/lcp-metadata-transformation-stylesheets/MARC to MODS/SelectedMarc.xml" htmlbaseurl="" outputurl="" processortype="saxon8"
		          useresolver="no" profilemode="0" profiledepth="" profilelength="" urlprofilexml="" commandline="" additionalpath="" additionalclasspath="" postprocessortype="none" postprocesscommandline="" postprocessadditionalpath=""
		          postprocessgeneratedext="" validateoutput="no" validator="internal" customvalidator="">
			<advancedProp name="bSchemaAware" value="false"/>
			<advancedProp name="xsltVersion" value="2.0"/>
			<advancedProp name="schemaCache" value="||"/>
			<advancedProp name="iWhitespace" value="0"/>
			<advancedProp name="bWarnings" value="true"/>
			<advancedProp name="bXml11" value="false"/>
			<advancedProp name="bUseDTD" value="false"/>
			<advancedProp name="bXsltOneIsOkay" value="true"/>
			<advancedProp name="bTinyTree" value="true"/>
			<advancedProp name="bGenerateByteCode" value="false"/>
			<advancedProp name="bExtensions" value="true"/>
			<advancedProp name="iValidation" value="0"/>
			<advancedProp name="iErrorHandling" value="fatal"/>
			<advancedProp name="sInitialTemplate" value=""/>
			<advancedProp name="sInitialMode" value=""/>
		</scenario>
	</scenarios>
	<MapperMetaTag>
		<MapperInfo srcSchemaPathIsRelative="yes" srcSchemaInterpretAsXML="no" destSchemaPath="" destSchemaRoot="" destSchemaPathIsRelative="yes" destSchemaInterpretAsXML="no"/>
		<MapperBlockPosition></MapperBlockPosition>
		<TemplateContext></TemplateContext>
		<MapperFilter side="source"></MapperFilter>
	</MapperMetaTag>
</metaInformation>
-->