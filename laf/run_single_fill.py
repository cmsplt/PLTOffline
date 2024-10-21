from src.model.searcher import AnomalySearcher
MOUNT_TARGET = "/eos/cms/store/group/dpg_bril/comm_bril/2022/physics/"
searcher = AnomalySearcher(MOUNT_TARGET)
FILL_N = 7921
result = searcher(FILL_N,return_preprocessed=True,generate_plots=True,save_path = "./results/22")
