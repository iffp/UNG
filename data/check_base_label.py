import sys
from collections import defaultdict

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python check_base_label.py <label_file>")
        sys.exit(1)
    label_file = sys.argv[1]
    
    label_to_frequency = defaultdict(int)
    try:
        with open(label_file, 'r') as f:
            for i, line in enumerate(f):
                # Check if line is empty
                if not line.strip():
                    print(f"Error: The label set of vector {i} is empty")
                    exit(-1)
                    
                # Split labels
                labels = line.strip().split(',')
                
                # Check if labels are numbers
                try:
                    labels = [int(x) for x in labels]
                except ValueError:
                    print(f"Error: The label set of vector {i} contains non-numeric labels")
                    exit(-1)
                
                # Check for duplicate labels
                if len(labels) != len(set(labels)):
                    print(f"Error: The label set of vector {i} has duplicate labels")
                    exit(-1)
                
                # Check label range
                for label in labels:
                    if label < 1:
                        print(f"Error: The label set of vector {i} has non-positive integer labels, label range should be [1,L] where L is total number of labels")
                        exit(-1)
                    
                # Calculate label frequency
                for label in labels:
                    label_to_frequency[label] += 1
        
    except Exception as e:
        print(f"Error reading file: {str(e)}")
        exit(-1)
    
    # Check label frequency
    label_and_frequency = sorted(label_to_frequency.items(), key=lambda x: x[0])
    for i in range(1, len(label_and_frequency)):
        if label_and_frequency[i][1] > label_and_frequency[i-1][1]:
            print(f"Warning: Label {label_and_frequency[i][0]} appears more frequently than label {label_and_frequency[i-1][0]} ({label_and_frequency[i][1]} > {label_and_frequency[i-1][1]}), please try to ensure smaller labels appear more frequently")
    print("Label file format is correct")