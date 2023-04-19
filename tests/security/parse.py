import pandas as pd

df = pd.read_csv("mantissas_total.csv")
df = df.drop(["Value"], axis=1)

print(f"Minimum likelihood: {df['Key'].min()/(2**23)}")
print(f"Maximum likelihood: {df['Key'].max()/(2**23)}")
print(f"Average likelihood: {df['Key'].mean()/(2**23)}")
print(f"Uniform likelihood: {1/2**23}")
