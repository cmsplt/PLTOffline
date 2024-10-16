from src.model.searcher import AnomalySearcher
MOUNT_TARGET = "./brildata/22/"
searcher = AnomalySearcher(MOUNT_TARGET)
FILL_N = 8010
result = searcher(FILL_N,return_preprocessed=True,generate_plots=True,save_path = "./results/22")
