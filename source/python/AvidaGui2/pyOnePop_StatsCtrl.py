# -*- coding: utf-8 -*-

from qt import *
from pyOnePop_StatsView import pyOnePop_StatsView


class pyOnePop_StatsCtrl(pyOnePop_StatsView):

  def __init__(self,parent = None,name = None,fl = 0):
    pyOnePop_StatsView.__init__(self,parent,name,fl)
        
  def construct(self, session_mdl):
    self.m_session_mdl = session_mdl
    self.m_avida = None
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("setAvidaSig"),
      self.setAvidaSlot)
    self.connect(
      self.m_session_mdl.m_session_mdtr, PYSIGNAL("orgClickedOnSig"),
      self.updateOrgReportSlot)
    self.clickedCellNumber = -99

  def setAvidaSlot(self, avida):
    old_avida = self.m_avida
    self.m_avida = avida
    if(old_avida):
      self.disconnect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)
      del old_avida
    if(self.m_avida):
      self.connect(
        self.m_avida.m_avida_thread_mdtr, PYSIGNAL("AvidaUpdatedSig"),
        self.avidaUpdatedSlot)

  def avidaUpdatedSlot(self):
    stats = self.m_avida.m_population.GetStats()
             
    #STATISTICS WINDOW
    string_output_length = 7

    if stats.GetAveFitness()<100000 : 
      avg_fitness = str(stats.GetAveFitness())
      string_length = len(avg_fitness)
      while string_length < string_output_length:
        avg_fitness = avg_fitness + '0'
        string_length = string_length+1
      self.m_avg_fitness.setText(avg_fitness[0:string_output_length])
    else:
      avg_fitness = "%.2g" %(stats.GetAveFitness())
      self.m_avg_fitness.setText(avg_fitness)


    dom_fitness = str(stats.GetDomFitness())
#    string_length = len(dom_fitness)
#    while string_length < string_output_length:
#      dom_fitness = dom_fitness + '0'
#      string_length = string_length+1
    self.m_dom_fitness.setText(dom_fitness[0:string_output_length])

    num_orgs = stats.GetNumCreatures()
    self.m_num_orgs.setText(QString("%1").arg(num_orgs))

    avg_gest = "%d" %(stats.GetAveGestation())
#    string_length = len(avg_gest)
#    while string_length < string_output_length:
#      avg_gest = avg_gest + '0'
#      string_length = string_length+1
    self.m_avg_gest.setText(QString("%1").arg(avg_gest))


    #TASK OUTLOOK 
    
    #if num_orgs_doing_a_given_task is above this number, we say the pop is doing this task
    m_org_threshold = 1   
 
    num_not = str(stats.GetTaskLastCount(0))
#    if num_not > m_org_threshold:
#      self.m_num_not.setText(QString("yes"))
#    else:
#      self.m_num_not.setText(QString("no"))
    self.m_num_not.setText(num_not)
    
    num_nand = str(stats.GetTaskLastCount(1))
#    if num_nand > m_org_threshold:
#      self.m_num_nand.setText(QString("yes"))
#    else:
#      self.m_num_nand.setText(QString("no"))
    self.m_num_nand.setText(num_nand)

    num_and = str(stats.GetTaskLastCount(2))
#    if num_and > m_org_threshold:
#      self.m_num_and.setText(QString("yes"))
#    else:
#      self.m_num_and.setText(QString("no"))
    self.m_num_and.setText(num_and)

    num_ornot = str(stats.GetTaskLastCount(3))
#    if num_ornot > m_org_threshold:
#      self.m_num_ornot.setText(QString("yes"))
#    else:
#      self.m_num_ornot.setText(QString("no"))
    self.m_num_ornot.setText(num_ornot)

    num_or = str(stats.GetTaskLastCount(4))
#    if num_or > m_org_threshold:
#      self.m_num_or.setText(QString("yes"))
#    else:
#      self.m_num_or.setText(QString("no"))
    self.m_num_or.setText(num_or)

    num_andnot = str(stats.GetTaskLastCount(5))
#    if num_andnot > m_org_threshold:
#      self.m_num_andnot.setText(QString("yes"))
#    else:
#      self.m_num_andnot.setText(QString("no"))
    self.m_num_andnot.setText(num_andnot)

    num_nor = str(stats.GetTaskLastCount(6))
#    if num_nor > m_org_threshold:
#      self.m_num_nor.setText(QString("yes"))
#    else:
#      self.m_num_nor.setText(QString("no"))
    self.m_num_nor.setText(num_nor)

    num_xor = str(stats.GetTaskLastCount(7))
#    if num_xor > m_org_threshold:
#      self.m_num_xor.setText(QString("yes"))
#    else:
#      self.m_num_xor.setText(QString("no"))
    self.m_num_xor.setText(num_xor)

    num_equals = str(stats.GetTaskLastCount(8))
#    if num_equals > m_org_threshold:
#      self.m_num_equals.setText(QString("yes"))
#    else:
#      self.m_num_equals.setText(QString("no"))
    self.m_num_equals.setText(num_equals)
    
    if self.clickedCellNumber>= 0: 
      self.updateOrgReportSlot(self.clickedCellNumber)



  def updateOrgReportSlot(self, clickedCellNum):
  
    self.clickedCellNumber = clickedCellNum
    
    clickedCell = self.m_avida.m_population.GetCell(int(clickedCellNum))

#    print "clickedCell.IsOccupied() returns " 
#    print clickedCell.IsOccupied()

    if not clickedCell.IsOccupied():
      #PAINT the stats fields empty
      self.m_org_name.setText('empty cell')
      self.m_org_fitness.setText('-')
#      self.m_cur_task_count.setText('-')
#      self.m_org_genome_length.setText('-')
      self.m_org_gestation_time.setText('-')
      self.m_org_age.setText('-')
      return
 
    organism = clickedCell.GetOrganism()
    phenotype = organism.GetPhenotype()
    genotype = organism.GetGenotype()

    m_org_fitness = phenotype.GetFitness()
    self.m_org_fitness.setText(QString("%1").arg(m_org_fitness))    

    m_org_name = str(genotype.GetName())
    hyphen_position = m_org_name.find('-')
    m_org_name = m_org_name[hyphen_position+1:]   

    self.m_org_name.setText(str(m_org_name))

#    self.m_org_name.setText(('-'))

#    m_cur_task_count = phenotype.GetCurTaskCount()
#    print "m_cur_task_count is "
#    print m_cur_task_count(1)

#    if we want to display length
#    m_org_genome_length = phenotype.GetGenomeLength()
#    print "m_org_genome_length is %f" %(m_org_genome_length)
#    self.m_org_genome_length.setText(QString("%1").arg(m_org_genome_length))

    m_org_gestation_time = phenotype.GetGestationTime()
    self.m_org_gestation_time.setText(QString("%1").arg(m_org_gestation_time))

    m_org_age = phenotype.GetAge()
    self.m_org_age.setText(QString("%1").arg(m_org_age))
    


