import ROOT as r

class lumidata():

	import os
	dictdir = os.getenv("VDMPATH")

	r.gROOT.LoadMacro(dictdir+"/dict/DIP_dict.C+")
	r.gROOT.LoadMacro(dictdir+"/dict/HLX_dict.C+")
	r.gROOT.LoadMacro(dictdir+"/dict/L1Trg_dict.C+")
	r.gROOT.LoadMacro(dictdir+"/dict/RCMSConf_dict.C+")


	# DIP data
	beam = [r.BEAM_INFO() for i in range(2)]
	dip = r.DIP_COMBINED_DATA()

	# HLX data
	summary = r.LUMI_SUMMARY()
	detail = r.LUMI_DETAIL()
	header = r.LUMI_SECTION_HEADER()
	occ = [r.OCCUPANCY_SECTION for i in range (36)]
	et = [r.ET_SUM_SECTION for i in range (36)]

	# LEVEL1 Trigger data
	l1trg = r.LEVEL1_TRIGGER()
	gtAlgo = [r.LEVEL1_PATH() for i in range(128)]
	gtTech = [r.LEVEL1_PATH() for i in range(64)]

	# RCMS Config
	rcms = r.RCMS_CONFIG()


class lumianalysis():

	data = lumidata()
		
	# Merged Files

	def initDIP(self):

		self.dip_t = self.rfile.Get("DIPCombined")
		self.dip_t.SetBranchAddress("DIPCombined.",r.AddressOf(self.data.dip))
		
	def initHLX(self):

		self.hlx_t = self.rfile.Get("HLXData")
		self.hlx_t.SetBranchAddress("Summary.",r.AddressOf(self.data.summary))
		self.hlx_t.SetBranchAddress("Detail.",r.AddressOf(self.data.detail))
		self.hlx_t.SetBranchAddress("Header.",r.AddressOf(self.data.header))
			
		for hlx in range(36):

			if hlx < 10:
				etstring = "ETSum0" + str(hlx) + "."
				occstring = "Occupancy0" + str(hlx) + "."
			else:
				etstring = "ETSum" + str(hlx) + "."
				occstring = "Occupancy" + str(hlx) + "."
		

			occ = r.OCCUPANCY_SECTION()
			et = r.ET_SUM_SECTION()

			self.hlx_t.SetBranchAddress(etstring,r.AddressOf(et))
			self.hlx_t.SetBranchAddress(occstring,r.AddressOf(occ))

			self.data.occ[hlx] = occ
			self.data.et[hlx] = et
		

	def initL1Trg(self):
		self.l1trg_t = self.rfile.Get("L1Trigger") 
		self.l1trg_t.SetBranchAddress("L1Trigger.",r.AddressOf(self.data.l1trg))

	def initRCMSConf(self):
		self.rcms_t = self.rfile.Get("RCMSConfig")
		self.rcms_t.SetBranchAddress("RCMSConfig.",r.AddressOf(self.data.rcms))

	# main init
	
	def init(self,filename):
		
		self.rfile = r.TFile(filename,'read')
		self.nLS = 0

		try: 
			self.initDIP()
			self.nLS = self.dip_t.GetEntries()	
			print "DIP data available, " + str(self.nLS)+ " sections"
		except AttributeError:
			print "No DIP data available"
			pass
		try:	
			self.initHLX()
			self.nLS = self.hlx_t.GetEntries()
			print "HLX data available, " + str(self.nLS) + " sections"
		except AttributeError:
			print "No HLX data available"
			pass
		try: 
			self.initL1Trg()
			print "L1Trigger data available"
		except AttributeError:
			pass
		try:	
			self.initRCMSConf()
			print "RCSMConfig data available"
		except AttributeError:
			print "No RCMSConfig data available"
			pass

	# VdM special files

	def initVdMDIP(self):

		self.dip_t = self.rfile.Get("VdMDIPCombined")
		self.dip_t.SetBranchAddress("VdMDIPCombined.",r.AddressOf(self.data.dip))

	def initVdMHLX(self):

		self.hlx_t = self.rfile.Get("VdMScanHLXData")
		self.hlx_t.SetBranchAddress("Summary.",r.AddressOf(self.data.summary))
		self.hlx_t.SetBranchAddress("Detail.",r.AddressOf(self.data.detail))
		self.hlx_t.SetBranchAddress("Header.",r.AddressOf(self.data.header))
			
		for hlx in range(36):

			if hlx < 10:
				etstring = "ETSum0" + str(hlx) + "."
				occstring = "Occupancy0" + str(hlx) + "."
			else:
				etstring = "ETSum" + str(hlx) + "."
				occstring = "Occupancy" + str(hlx) + "."
		

			occ = r.OCCUPANCY_SECTION()
			et = r.ET_SUM_SECTION()

			self.hlx_t.SetBranchAddress(etstring,r.AddressOf(et))
			self.hlx_t.SetBranchAddress(occstring,r.AddressOf(occ))

			self.data.occ[hlx] = occ
			self.data.et[hlx] = et

	def initVdM(self,filename):

		self.rfile = r.TFile(filename,'read')
                self.nLS = 0

                try:
                        self.initVdMDIP()
                        self.nLS = self.dip_t.GetEntries()
                        print "VdMDIP data available, " + str(self.nLS)+ " sections"
                except AttributeError:
                        print "No VdMDIP data available"
                        pass
                try:    
                        self.initVdMHLX()
                        self.nLS = self.hlx_t.GetEntries()
                        print "HLX data available, " + str(self.nLS) + " sections"
                except AttributeError:
                        print "No HLX data available"
                        pass
		try: 
			self.initL1Trg()
			print "L1Trigger data available"
		except AttributeError:
			pass
		try:	
			self.initRCMSConf()
			print "RCSMConfig data available"
		except AttributeError:
			print "No RCMSConfig data available"
			pass
	
	def loadLS(self,ls):
		
		if self.nLS == 0 : return
		try:
			self.hlx_t.GetEntry(ls)
		except AttributeError:
			pass
		try:
			self.dip_t.GetEntry(ls)
			for i in range(2):
				r.getBeam(self.data.dip,self.data.beam[i],i)
		except AttributeError:
			pass
		try:
			self.l1trg_t.GetEntry(ls)
			for i in range(128):
				r.getL1PathAlgo(self.data.l1trg,self.data.gtAlgo[i],i)
			for i in range(64):
				r.getL1PathTech(self.data.l1trg,self.data.gtTech[i],i)
		except AttributeError:
			pass
		try:	
			self.rcms_t.GetEntry(ls)
		except AttributeError:
			pass

