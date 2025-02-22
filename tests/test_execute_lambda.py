async def test_execute_lambda(service_client):
    response = await service_client.post('/v1/execute/lambda/125')
    assert response.status == 200
    assert response.content == b'125'
