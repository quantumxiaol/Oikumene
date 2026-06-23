from fastapi import FastAPI

from oikumene_agent.api.routes import router


app = FastAPI(
    title="Oikumene Agent Server",
    version="0.1.0",
    description="Advisory strategic decision service for Oikumene.",
)

app.include_router(router)
